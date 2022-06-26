# -*- coding: utf-8 -*-
import sys
import time
import logging
import signal
import numpy as np
import wiringpi  # type: ignore
from typing import List

from hardware import AutomatrixHardware

from typing import List, Tuple


class AutomatrixStream:

    def __init__(self):
        # store byte encoded patterns in an ordered list
        self.patterns = []

    def clear(self) -> None:
        self.patterns.clear()

    def set_pattern(self, pattern: np.ndarray):
        """
        Set the next pattern in this automatrix stream sequence
        """
        encoded_pattern = self._encode(pattern)
        self.patterns.append(encoded_pattern)

    def _encode(self, pattern: np.ndarray) -> np.ndarray:
        """
        Perform all validation checks on data integrity before encoding the np.array pattern into a
        format that the Automatrix can interpret.
        :param pattern: construction format
        :return: byte encoded format
        """
        if not ((pattern.dtype == np.uint8) or (pattern.dtype == np.bool_)):
            raise TypeError("Elements of pattern must be of dtype np.uint8 or np.bool_")
        if np.shape(pattern) != (AutomatrixHardware.MATRIX_DIM, AutomatrixHardware.MATRIX_DIM):
            raise ValueError("Unreadable matrix shape. Expected: {} but received {}".format(
                (AutomatrixHardware.MATRIX_DIM, AutomatrixHardware.MATRIX_DIM),
                np.shape(pattern)))
        if (np.count_nonzero(pattern.flatten() == 1) > 164):
            raise ValueError("Exceeds power budget")
        new_pattern = self._pattern_to_bytes(pattern)
        # post-encoding sanity checks
        if len(new_pattern) != AutomatrixHardware.SHIFTERS or \
                (np.array(new_pattern) > 255).any() or (np.array(new_pattern) < 0).any():
            raise ValueError("Pattern length invalid")
        return new_pattern

    def _pattern_to_bytes(self, pattern: np.ndarray) -> np.ndarray:
        """
        Partitions a 16x16 pattern into an set of arrays of 32 bytes.
        :param pattern: pattern (np.array): 16x16 array of bools_ or ints (1 or 0)
        :return: byte_pattern (np.array): 32x1 array of bytes (uint8 has 8 bits per byte)
        """
        # deconstruct the pattern to their shift "blocks"
        temp_pattern = np.empty(
            (AutomatrixHardware.SHIFTERS, AutomatrixHardware.PARALLEL_OUT), dtype=np.bool_)
        for col_idx in range(AutomatrixHardware.MATRIX_DIM):
            for row_idx in range(AutomatrixHardware.MATRIX_DIM):
                # change coordinate system
                shift_idx, LED_idx = self._grid_to_shift_coords(col_idx, row_idx)
                # the modified index due to hardware routing
                LED_idx = AutomatrixHardware.LED_MAP[LED_idx]
                temp_pattern[shift_idx, LED_idx] = pattern[col_idx, row_idx]
        # fresh array of bytes (one byte per shift register)
        byte_pattern = np.empty(AutomatrixHardware.SHIFTERS, dtype=np.uint8)
        # convert to bytes and populate
        for shift_idx in range(AutomatrixHardware.SHIFTERS):
            # list of 8bits converted to a , ex. [0,0,0,0,0,0,1,1] = 3
            as_bytes = np.packbits(temp_pattern[shift_idx, :])
            byte_pattern[shift_idx] = as_bytes
        return byte_pattern

    def get_pattern_as_array(self) -> List[np.ndarray]:
        """
        Convert all byte patterns in the sequence to a human readable grid array.
        :return: the sequence of patterns as a list of np.array(s)
        """
        converted_patterns = list()
        for pattern in self.patterns:
            array_pattern = np.empty((AutomatrixHardware.MATRIX_DIM,
                                     AutomatrixHardware.MATRIX_DIM), dtype=np.bool_)
            for shift_idx in range(AutomatrixHardware.SHIFTERS):
                # list of 8 bits converted to a , ex. [0,0,0,0,0,0,1,1] = 3
                byte_as_list = np.unpackbits(pattern[shift_idx])
                for LED_idx in range(AutomatrixHardware.PARALLEL_OUT):
                    row_idx, col_idx, = self._shift_to_grid_coords(shift_idx, LED_idx)
                    array_pattern[row_idx,
                                  col_idx] = byte_as_list[AutomatrixHardware.LED_MAP[LED_idx]]
            converted_patterns.append(array_pattern)
        return converted_patterns

    @staticmethod
    def _grid_to_shift_coords(col_idx: int, row_idx: int) -> Tuple[int, int]:
        """
        Convert coordinate system
        Convert from (16x16) grid coordinate system to 32x8 shift register
        coordinate system
        This is where the shift registers are placed on a 8x4 grid:
        7, 15, 23, 31,
        6, 14, 22, 30,
        5, 13, 21, 29,
        4, 12, 20, 28,
        3, 11, 19, 27,
        2, 10, 18, 26,
        1, 9,  17, 25,
        0, 8,  16, 24,
        Each shifter can will light a 2x4 block of LEDS.
        The above shifter pattern will produce a 16x16 LED grid.
        This functions returns the shift_idx when the indices in the
        coordinate system of the 16x16 grid, are used as arguments.
        This is where the LEDs are supposed to be placed on a 2x4 block:
         4,  5,  6,  7
         0,  1,  2,  3
        These (2x4) blocks of a shift register are layered in a way (8x4)
        to create the full 16x16 array.
        For example, `_grid_to_shift_coords(7, 3)`` will return `shift_idx=5` and `LED_idx=7`
        Args:
            col_idx (`int`): column index on the 16x16 led coordinate system
            row_idx (`int`): row index on the 16x16 led coordinate system
        Returns:
            shift_idx (`int`): the index of the shift register that
            supports the given LED.
            LED_idx (`int`): the index of the LED  in the register.
        """
        shift_idx = (row_idx // 2) + (col_idx // 4) * 8
        LED_idx = col_idx % 4 + (row_idx % 2) * 4
        return shift_idx, LED_idx

    @staticmethod
    def _shift_to_grid_coords(shift_idx: int, LED_idx: int) -> Tuple[int, int]:
        """
        Convert coordinate system
        Convert from 32x8 shift register coordinate system to
        16x16 grid coordinate system.
        This is where the shift registers are placed on a 8x4 grid:
        7, 15, 23, 31,
        6, 14, 22, 30,
        5, 13, 21, 29,
        4, 12, 20, 28,
        3, 11, 19, 27,
        2, 10, 18, 26,
        1, 9,  17, 25,
        0, 8,  16, 24,
        Each shifter can will light a 2x4 block of LEDS.
        The above shifter pattern will produce a 16x16 LED grid.
        This functions returns the shift_idx when the indices in the
        coordinate system of the 16x16 grid, are used as arguments.
        This is where the LEDs are supposed to be placed on a 2x4 block:
         4,  5,  6,  7
         0,  1,  2,  3
        These (2x4) blocks of a shift register are layered in a way (8x4)
        to create the full 16x16 array.
        For example, `_grid_to_shift_coords(7, 3)`` will return `shift_idx=5` and `LED_idx=7`
        Args:
            col_idx (`int`): column index on the 16x16 led coordinate system
            row_idx (`int`): row index on the 16x16 led coordinate system
        Returns:
            shift_idx (`int`): the index of the shift register that
            supports the given LED.
            LED_idx (`int`): the index of the LED  in the register.
        """
        col_idx = (shift_idx // 8) * 4 + LED_idx % 4
        row_idx = (shift_idx % 8) * 2 + LED_idx // 4
        return col_idx, row_idx


class Automatrix:

    def __init__(self):
        self._logger = logging.getLogger(__name__)
        self.dpc = AutomatrixStream()
        self.focus = AutomatrixStream()
        # setup wiringPi SPI
        self.outer_radius = 4
        self.inner_radius = 3
        wiringpi.wiringPiSetupGpio()
        wiringpi.pinMode(AutomatrixHardware.TRIGGER, wiringpi.OUTPUT)
        wiringpi.wiringPiSPISetup(AutomatrixHardware.SPI_CHANNEL, AutomatrixHardware.SPI_SPEED)
        self._logger.info("Instantiation successful.")

    def configure(self):
        # clear previous patterns
        self.focus.clear()
        self.dpc.clear()
        self.enable(False)
        # compute focus patterns
        focus_pattern = self._generate_focus_pattern(self.outer_radius, self.inner_radius)
        # Add the focus pattern to the stream map
        self.focus.set_pattern(focus_pattern)
        # compute focus patterns
        dpc_patterns = self._generate_dpc_patterns(self.outer_radius, self.inner_radius)
        # create and configure new automatrix stream
        for pattern in dpc_patterns:
            pattern_print = pattern.astype(np.uint8)
            self._logger.debug("Added new dpc pattern: \n%s", pattern_print)
            self.dpc.set_pattern(pattern)
        # Add the dpc patterns to the stream map
        self._logger.debug("Config successful.")

    def load_pattern(self, pattern: List[int]):
        """
        Load the specified automatrix stream data through the spi interface.
        """
        wiringpi.wiringPiSPIDataRW(AutomatrixHardware.SPI_CHANNEL, bytes(pattern.copy()))
        self._logger.debug("Pattern loaded: %s", bytes(pattern))

    def enable(self, active: bool):
        """
        Write high or low to the automatrix trigger pin
        :param active: True is activate; False is deactivate
        """
        wiringpi.digitalWrite(AutomatrixHardware.TRIGGER, int(
            active ^ AutomatrixHardware.ACTIVE_HIGH))
        self._logger.debug("Automatrix enable: %s", int(active ^ AutomatrixHardware.ACTIVE_HIGH))

    @staticmethod
    def _generate_dpc_patterns(outer_radius: float, inner_radius: float) -> List[np.ndarray]:
        """
        Computes the DPC LED patterns based on the selected dialation sizes. This results in
        four half circle patterns corresponding to the cardinal directions in sequence: upper, lower,
        left, right.
        """
        # tweak to center the circle
        ox, oy = AutomatrixHardware.MATRIX_DIM / 2 - 0.5, AutomatrixHardware.MATRIX_DIM / 2 - 0.5
        or_sq, ir_sq = outer_radius**2, inner_radius**2
        shape = (AutomatrixHardware.MATRIX_DIM, AutomatrixHardware.MATRIX_DIM)
        upper_px = np.zeros(shape, dtype=np.bool_)
        lower_px = np.zeros(shape, dtype=np.bool_)
        left_px = np.zeros(shape, dtype=np.bool_)
        right_px = np.zeros(shape, dtype=np.bool_)
        for i in np.arange(AutomatrixHardware.MATRIX_DIM):
            for j in np.arange(AutomatrixHardware.MATRIX_DIM):
                if not ((i - ox)**2 + (j - oy)**2 > or_sq):
                    if ((i - ox)**2 + (j - oy)**2 > ir_sq):
                        if i > ox:
                            left_px[i, j] = 1
                        if i < ox:
                            right_px[i, j] = 1
                        if j > oy:
                            upper_px[i, j] = 1
                        if j < oy:
                            lower_px[i, j] = 1
        return [upper_px, lower_px, left_px, right_px]

    @staticmethod
    def _generate_focus_pattern(outer_radius: float, inner_radius: float) -> np.ndarray:
        """
        Computes one DPC LED pattern based on the selected dialation sizes. This results in one half
        circle pattern of arbitrary direction.
        """
        left_px = np.zeros((AutomatrixHardware.MATRIX_DIM,
                           AutomatrixHardware.MATRIX_DIM), dtype=np.bool_)
        ox, oy = AutomatrixHardware.MATRIX_DIM / 2 - \
            0.5, AutomatrixHardware.MATRIX_DIM / 2 - 0.5   # tweak to center the circle
        or_sq, ir_sq = outer_radius**2, inner_radius**2
        for i in np.arange(AutomatrixHardware.MATRIX_DIM):
            for j in np.arange(AutomatrixHardware.MATRIX_DIM):
                rad_sqr_ij = (i - ox)**2 + (j - oy)**2
                if ir_sq < rad_sqr_ij < or_sq:
                    if i > ox:
                        left_px[i, j] = 1
        return left_px


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    automtx = Automatrix()
    automtx.configure()
    while True:
        for pattern in automtx.dpc.patterns:
            # load the first pattern into the shift registers
            automtx.load_pattern(pattern)
            automtx.enable(False)
            time.sleep(1)
            automtx.enable(True)
