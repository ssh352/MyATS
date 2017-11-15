import os
import sys
import string
import re

os.system('find ./ -name *.lib|xargs rm')
os.system('find ./ -name *.dll|xargs rm')
os.system("find ./ -name *.so*|xargs rm")
