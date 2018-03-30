# sphinx-small

For Linux / Rasbian Platform:

# To compile:

cd src

./mk

cd ../test

./mk

# To test speech to text:

In one command line window -

cd test

./run-sphinx

In another command line window -

cd test , use a USB mic:

./vad.exe

speak to mic, see results in audio.txt

# To test speech to text:

In one command line window -

cd test

./run-sphinx-kwd

In another command line window -

cd test , use a USB mic:

./vad.exe

speak anything plus keywords (e.g. defined in trigger.jsgf) to mic, see results in audio.txt




