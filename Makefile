all:
	mkdir -p ./build/libs
	mkdir -p ./build/bin
	make -C ./network
	make -C ./samples

clean:
	make -C ./network clean
	make -C ./samples clean
