CXX = clang++
CXXFLAGS =-std=c++11 -pthread -stdlib=libstdc++ -D_WEBSOCKETPP_CPP11_STL_ -D_WEBSOCKETPP_NO_CPP11_REGEX_ -Waddress-of-temporary
Active_Flags =
D_Flags = -g
O_Flags = -g
WS_Link = -lpthread -lcrypto -D LWS_OPENSSL_SUPPORT -lssl
DB_Link	= -lmysqlcppconn
B_Link = -lboost_regex -lboost_system -lboost_chrono -lboost_date_time -ljsoncpp
Obj_Dir = objs/
Out_Dir = bin/
BUILD_NUM = counter



default: debug
debug: Active_Flags = $(D_Flags)
optimized: Active_Flags = $(O_Flags)
start: forum http main.o
	@echo "Compiling"
	$(CXX) $(CXXFLAGS) src/forum/*.o src/http/*.o  main.o -o serv $(Active_Flags) $(WS_Link) $(DB_Link) $(B_Link) -v


debug: start
optimized: start
forum: src/forum/uint256.o src/forum/util.o  src/forum/Forum.o src/forum/Groups.o
http: src/http/database.o src/http/http.o

clean:
	@echo "======Cleaning===="
	rm -f src/*/*.o
	rm -f *.o
	rm -f src/*/*.P
	rm -f *.P
	rm -f serv

%.o : %.cpp %.h
	@echo "======[Building $<]======"
	@$(CXX) $(CXXFLAGS) -MD -c -o $@ $< $(Active_Flags)
	@cp $*.d $*.P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
		rm -f $*.d

-include *.P */*.P
