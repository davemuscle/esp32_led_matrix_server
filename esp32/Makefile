
SERIAL=/dev/ttyUSB0

.PHONY: header
header: top/html_static.h
top/html_static.h: index.html
	echo -n "#define HTML_BODY \"" > $@
	sed '/PARSE_PATTERN/Q' $^ | htmlmin | sed 's/"/\\"/g' | awk '{$$1=$$1;print}' | tr -d '\n' >> $@
	echo "\"" >> $@

.DEFAULT_GOAL = prog
.PHONY: prog
prog: header
	arduino-cli compile -b esp32:esp32:esp32 $(addprefix --library ,$(wildcard lib/*)) top
	arduino-cli upload top -p ${SERIAL} --fqbn esp32:esp32:esp32 --board-options UploadSpeed=921600,FlashMode=qio,LoopCore=1,EventsCore=1

.PHONY: fuse
fuse:
	espefuse.py set_flash_voltage 3.3V

.PHONY: screen
screen:
	minicom -D $(SERIAL)

.PHONY: clean
clean:
	rm top/html_static.h

