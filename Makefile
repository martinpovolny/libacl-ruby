build-stamp:
clean:
install:
	@echo installing data to the dir: '$(DESTDIR)'
	if [ -n "$(DESTDIR)" ] ; then cp -a root/* "$(DESTDIR)" ; fi
