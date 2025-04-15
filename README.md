# ctoml-embedded
An easier to use and highly-portable TOML C library.

This library only use a small internal buffer and a given string with the TOML
data, and no dynamic memory is needed in any way.

## Description
__Attention__: This is an early alpha WIP project. Currently is non-functional

### What is done:
- Primary parsing the structure of the content
- Basic format error checking
- Most helper functions to getting the rest of the values
- Getting booleans from the TOML data

### What needs to be done
- Getting the different types of values
- Testing on different platforms
- Adding more testing cases
- Converting UTF-16 to UTF-8 when processing strings

## License

MIT License

Copyright (c) 2025 José Miguel Rodríguez Marchena (josemirm)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
