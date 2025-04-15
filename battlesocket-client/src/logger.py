import logging

class Logger:

    def __init__(self,filename):
        logging.basicConfig(filename=filename, filemode="a", level=logging.DEBUG,
                            format='%(asctime)s %(message)s', datefmt='%Y-%m-%d %I:%M:%S ')

            

