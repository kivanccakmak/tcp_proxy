#!/bin/bash
up_count=1
response='['
pre_line=''
pre_key=''
pre_val=''
cur_line=''
cur_key=''
cur_val=''
info=''

dmesg_data=$(cat input.txt | grep -E 'bw|mac')
data_len=$(echo "$dmesg_data" | wc -l)
data_len=$((data_len))

for ((i=1; i<=$data_len; i++))
do
    pre_line=''
    cur_line=''
    pre_line=$(cat input.txt | grep -E 'bw|mac' | head -$up_count | tail -1)
    ((up_count++))
    cur_line=$(cat input.txt | grep -E 'bw|mac' | head -$up_count | tail -1)
    pre_key=${pre_line%%' '*}
    pre_val=${pre_line#*' '}
    cur_key=${cur_line%%' '*}
    cur_val=${cur_line#*' '}
    if [ "$pre_key" = "sniffed_mac" ]
    then
        if [ "$cur_key" = "sniffed_bw" ]
        then
            info=''
            info='{mac:'$pre_val',bw:'$cur_val'}'
            response=$response$info','
        fi
    fi
done
response="${response::-1}"
response='"'$response']''"'
echo $response
