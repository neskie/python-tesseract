from distutils.core import setup,Extension
import os
os.environ['CC'] = 'g++'

setup(name = "PyTesseract",
      version = "0.5",
      description = "Bindings for the tesseract ocr",
      author = "Neskie Manuel",
      author_email = "neskiem@gmail.com",
      url = "None",
      license = 'Apache License',
      long_description = '''
This provides the necssary python bindings to call tesseract
within Python. In conjunction with Image Processing libraries,
this can be used to expand the capabilities.
''',

      ext_modules = [Extension(
              "tesseract", ["tesseract.cc"],
              libraries=["tesseract_full","tiff"]
              )])


