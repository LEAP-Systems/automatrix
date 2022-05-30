# -*- coding: utf-8 -*-
import sys
import time
import logging
import signal
import wiringpi  # type: ignore
from typing import List

from hardware import AutomatrixHardware

class Automatrix:

    def __init__(self):
        self._logger = logging.getLogger(__name__)
        # setup wiringPi SPI
        wiringpi.wiringPiSetupGpio()
        wiringpi.pinMode(AutomatrixHardware.TRIGGER, wiringpi.OUTPUT)
        wiringpi.wiringPiSPISetup(AutomatrixHardware.SPI_CHANNEL, AutomatrixHardware.SPI_SPEED)
        self._logger.info("Instantiation successful.")

    def disable(self):
        """
        Deactivate and shift in low bits
        """
        self.load_pattern(pattern=[0] * 32)  # equivalent to SRCLR
        self.trigger(False)

    def enable(self):
        """
        Deactivate and shift in low bits before dereferencing automatrix
        """
        self.load_pattern(pattern=[8] * 32)  # equivalent to SRCLR

    def load_pattern(self, pattern: List[int]):
        """
        Load the specified automatrix stream data through the spi interface.
        """
        wiringpi.wiringPiSPIDataRW(AutomatrixHardware.SPI_CHANNEL, bytes(pattern.copy()))
        self._logger.debug("Pattern loaded: %s", bytes(pattern))

    def trigger(self, active: bool):
        """
        Write high or low to the automatrix trigger pin

        :param active: True is activate; False is deactivate
        """
        wiringpi.digitalWrite(AutomatrixHardware.TRIGGER, int(active ^ AutomatrixHardware.ACTIVE_HIGH))
        self._logger.debug("Automatrix enable: %s", active)

    def release(self, signum: int, _):
        self._logger.info("Received signal: %s", signum)
        self.disable()
        self._logger.info("Released hardware")
        sys.exit(signum)


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    automtx = Automatrix()
    signal.signal(signal.SIGINT, automtx.release)
    signal.signal(signal.SIGTERM, automtx.release)
    automtx.enable()

    while True:
        automtx.trigger(True)
        time.sleep(1)
        automtx.trigger(False)
        time.sleep(1)