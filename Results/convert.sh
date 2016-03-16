#/bin/bash

convert -background "#222222" -alpha remove -density ${2:-200} ${1} ${1}.png

eog ${1}.png