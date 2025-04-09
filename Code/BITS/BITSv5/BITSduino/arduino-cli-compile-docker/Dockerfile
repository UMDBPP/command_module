FROM python:3.7-slim

RUN apt-get update && apt-get install -y curl
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/bin sh
RUN arduino-cli core update-index --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
RUN arduino-cli core install rp2040:rp2040 --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json 

RUN pip install --no-cache-dir pyyaml
COPY compile.py /usr/src/app/

WORKDIR /usr/src/sketch
CMD [ "python", "-u", "/usr/src/app/compile.py" ]
