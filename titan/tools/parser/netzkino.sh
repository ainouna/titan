rm cache.*
rm -rf _full/netzkino
mkdir -p _full/netzkino/streams
piccount=0

SEARCHLIST="81 61 39 1 4 32 18 6 51 31 3 10 5 33 34 71" 
     
for SEARCH in $SEARCHLIST; do
	echo SEARCH=$SEARCH 

	if [ $SEARCH == "81" ]; then
		TITLE="Neu bei Netzkino"
		filename="neu_bei_netzkino"
	elif [ $SEARCH == "61" ]; then
		TITLE="HD-Kino"
		filename="hd_kino"
	elif [ $SEARCH == "39" ]; then
		TITLE="Starkino"
		filename="starkino"
	elif [ $SEARCH == "4" ]; then
		TITLE="Dramakino"
		filename="dramakino"
	elif [ $SEARCH == "1" ]; then
		TITLE="Actionkino"
		filename="actionkino"
	elif [ $SEARCH == "32" ]; then
		TITLE="Thrillerkino"
		filename="thrillerkino"
	elif [ $SEARCH == "18" ]; then
		TITLE="Liebesfilmkino"
		filename="liebesfilmkino"
	elif [ $SEARCH == "6" ]; then
		TITLE="Scifikino"
		filename="scifikino"
	elif [ $SEARCH == "51" ]; then
		TITLE="Arthousekino"
		filename="arthousekino"
	elif [ $SEARCH == "31" ]; then
		TITLE="Queerkino"
		filename="queerkino"
	elif [ $SEARCH == "3" ]; then
		TITLE="Spasskino"
		filename="spasskino"
	elif [ $SEARCH == "10" ]; then
		title="Asiakino"
		filename="asiakino"
	elif [ $SEARCH == "5" ]; then
		TITLE="Horrorkino"
		filename="horrorkino"
	elif [ $SEARCH == "33" ]; then
		TITLE="Klassikerkino"
		filename="klassikerkino"
	elif [ $SEARCH == "34" ]; then
		TITLE="Westernkino"
		filename="westernkino"
	elif [ $SEARCH == "71" ]; then
		TITLE="Kino ab 18"
		filename="kino_ab_18"
	else
		filename=not_found	
	fi

	piccount=`expr $piccount + 1`
	URL="http://atemio.dyndns.tv/mediathek/netzkino/streams/netzkino.$filename.list"
	PIC="http://atemio.dyndns.tv/mediathek/menu/$filename.jpg"
	LINE="$TITLE#$URL#$PIC#netzkino_$piccount.jpg#Netzkino#0"
	if [ ! -z "$TITLE" ]; then
		echo $LINE >> cache.netzkino.category.titanlist	
	fi

	TITLE=""
																						
	rm cache.top_rated.list
	wget --no-check-certificate "http://www.netzkino.de/capi/get_category_posts?count=500&id=$SEARCH&custom_fields=Streaming" -O cache.$filename.list

	LIST=`cat cache.$filename.list | sed 's/}}\+/\n/g' | grep '"custom_fields":{"Streaming"'`

	count=0
	for ROUND in $LIST; do
		count=`expr $count + 1`
		piccount=`expr $piccount + 1`
		PIC=`echo $ROUND | sed 's/"thumbnail":/\nthumbnail:/' | grep ^thumbnail: | cut -d'"' -f2 | tr '\\' ' ' | sed 's/ \+//g'`
		TITLE=`echo $PIC | tr '/' '\n' | tail -n1 | sed 's/-172x198.jpg//' | sed 's/-172x198.gif//' | tr '-' ' ' | tr '_' ' '`
		URL=`echo $ROUND | sed 's/"Streaming":/\nStreaming:/' | grep ^Streaming: | cut -d'"' -f2 | tr '\\' ' ' | sed 's/ \+//g'`
		
		LINE="$TITLE#rtmp://mf.netzkino.c.nmdn.net/netzkino/_definst_/mp4:$URL#$PIC#netzkino_$piccount.jpg#Netzkino#2"
		if [ ! -z "$TITLE" ] && [ ! -z "$URL" ] && [ $count = 1 ]; then
			echo "$LINE" >> cache.netzkino.$filename.titanlist
			echo "$LINE" >> cache.netzkino.titanlist
		elif [ ! -z "$TITLE" ] && [ ! -z "$URL" ] && [ `cat cache.netzkino.$filename.titanlist | grep ^"$TITLE" | wc -l` -eq 0 ]; then
			echo "$LINE" >> cache.netzkino.$filename.titanlist
			echo "$LINE" >> cache.netzkino.titanlist
		fi
	done

	cat cache.netzkino.$filename.titanlist > _full/netzkino/streams/netzkino.$filename.list
done

cat cache.netzkino.titanlist | sort -m > _full/netzkino/streams/netzkino.all-newfirst.list
cat cache.netzkino.titanlist | sort -um > _full/netzkino/streams/netzkino.all-sorted.list
cat cache.netzkino.category.titanlist | sort -m > _full/netzkino/netzkino.category.list

for ROUND in 0 1 2 3 4 5 6 7 8 9 A B C D E F G H I J K L M N O P Q R S T U V W X Y Z; do
	filename=`echo "$ROUND" | tr 'A-Z' 'a-z'`
	if [ `cat cache.netzkino.titanlist | grep ^"$ROUND" | wc -l` -gt 0 ];then
		cat cache.netzkino.titanlist | grep ^"$ROUND" > cache.netzkino.titanlist."$ROUND"
		cat cache.netzkino.titanlist."$ROUND" | sort -um > _full/netzkino/streams/netzkino.`echo "$ROUND" | tr 'A-Z' 'a-z'`.list
		echo `echo "$ROUND" | tr 'A-Z' 'a-z'`"#http://atemio.dyndns.tv/mediathek/netzkino/streams/netzkino."`echo "$ROUND" | tr 'A-Z' 'a-z'`".list#http://atemio.dyndns.tv/mediathek/menu/`echo "$ROUND" | tr 'A-Z' 'a-z'`.jpg#"`echo "$ROUND" | tr 'A-Z' 'a-z'`.jpg#Netzkino#0 >> _full/netzkino/netzkino.a-z.list
	elif [ `cat cache.netzkino.titanlist | grep ^"$filename" | wc -l` -gt 0 ];then
		cat cache.netzkino.titanlist | grep ^"$ROUND" > cache.netzkino.titanlist."$ROUND"
		cat cache.netzkino.titanlist."$ROUND" | sort -um > _full/netzkino/streams/netzkino.`echo "$ROUND" | tr 'A-Z' 'a-z'`.list
		echo `echo "$ROUND" | tr 'A-Z' 'a-z'`"#http://atemio.dyndns.tv/mediathek/netzkino/streams/netzkino."`echo "$ROUND" | tr 'A-Z' 'a-z'`".list#http://atemio.dyndns.tv/mediathek/menu/`echo "$ROUND" | tr 'A-Z' 'a-z'`.jpg#"`echo "$ROUND" | tr 'A-Z' 'a-z'`.jpg#Netzkino#0 >> _full/netzkino/netzkino.a-z.list
	fi
done
rm cache.*
#cp -a _full/netzkino/* /var/www/atemio/web/mediathek/netzkino