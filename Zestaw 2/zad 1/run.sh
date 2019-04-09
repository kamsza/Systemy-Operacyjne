for record_size in 2 4 512 1024 4096 8192
do
	for record_number in 5000 10000
	do 
		./main generate dane $record_number $record_size
		./main copy dane dane2 $record_number $record_size  sys  
		./main copy dane dane3 $record_number $record_size  lib 
		./main sort dane2 $record_number $record_size sys 
		./main sort dane3 $record_number $record_size lib 
		rm -f dane dane2 dane3 ;
	done 
	echo -e "\n ==================================================================== \n"
done
