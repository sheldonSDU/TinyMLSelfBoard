# 测试加速度传感器
import unittest
from accelator import Accelator

class TestAccelator(unittest.TestCase):
    def test_accelator(self):
        accelator = Accelator()
        self.assertEqual(accelator.get_accel(), (0, 0, 0))