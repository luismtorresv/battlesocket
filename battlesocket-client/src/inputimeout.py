"""
We modified this person's code:

__title__ = 'inputimeout'
__description__ = 'Multi platform standard input with timeout'
__url__ = 'http://github.com/johejo/inputimeout'
__version__ = '1.0.4'
__author__ = 'Mitsuo Heijo'
__author_email__ = 'mitsuo_h@outlook.com'
__license__ = 'MIT'
__copyright__ = 'Copyright 2018 Mitsuo Heijo'
"""

"""
MIT License

Copyright (c) 2017 Mitsuo Heijo

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
"""

import selectors
import sys
import termios

LF = "\n"


class SocketReadAvailable(Exception):
    pass


class TimeoutOcurred(Exception):
    pass


def echo(string):
    sys.stdout.write(string)
    sys.stdout.flush()


def inputimeout(sock, prompt):
    echo(prompt)

    sel = selectors.DefaultSelector()
    stdin_key = sel.register(sys.stdin, selectors.EVENT_READ)
    sock_key = sel.register(sock, selectors.EVENT_READ)

    # A minute of timeout in case the user doesn't type in
    # anything _and_ the server somehow forgets about it?
    events = sel.select(timeout=60)

    if events:
        # Search for the socket first.
        sel_keys = [k for k, _ in events]
        if sock_key in sel_keys:
            sel.close()
            raise SocketReadAvailable
        if stdin_key in sel_keys:
            sel.close()
            return sys.stdin.readline().rstrip(LF)
    else:
        echo(LF)
        termios.tcflush(sys.stdin, termios.TCIFLUSH)
        sel.close()
        raise TimeoutOcurred
