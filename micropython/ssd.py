import machine
import ssd1306
i2c = machine.SoftI2C(scl=machine.Pin(22), sda=machine.Pin(21))
oled = ssd1306.SSD1306_I2C(128, 64, i2c)
oled.text('hello world!', 0, 10)
oled.show()
