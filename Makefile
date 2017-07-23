all:
	make -C ./network
	make -C ./samples

clean:
	make -C ./network clean
	make -C ./samples clean
