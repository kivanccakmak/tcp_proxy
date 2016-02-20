#!/bin/bash
up_count=1
down_val=1
start=1

pre_line=''
pre_key=''
pre_val=''
cur_line=''
cur_key=''
cur_val=''

dmesg_data=$(cat input.txt | grep -E 'bw|mac')
data_len=$(echo "$dmesg_data" | wc -l)
data_len=$((data_len))

for ((i=$start; i<=$data_len; i++))
do
    pre_line=''
    cur_line=''
    pre_line=$(cat input.txt | grep -E 'bw|mac' | head -$up_count | tail -$down_val)
    ((up_count++))
    cur_line=$(cat input.txt | grep -E 'bw|mac' | head -$up_count | tail -$down_val)
    pre_key=${pre_line%%' '*}
    pre_val=${pre_line#*' '}
    cur_key=${cur_line%%' '*}
    cur_val=${cur_line#*' '}
    #echo "pre_key: "$pre_key
    #echo "pre_val: "$pre_val
    #echo "cur_key: "$cur_key
    #echo "cur_val: "$cur_val
    if [ "$pre_key" = "sniffed_mac" ]
    then
        if [ "$cur_key" = "sniffed_bw" ]
        then
            echo "mac: "$pre_val" ""bw: "$cur_val
        fi
    fi
done



#get mac address
#get bandwidth
#add2_out_json


