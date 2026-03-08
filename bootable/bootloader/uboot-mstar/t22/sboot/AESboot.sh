AESbootIn=$1

AESboot=$(xxd -l 16 -p $AESbootIn)
hashkey=D29E2BAF64CA9A064E943567F78E047B
Result=""

for (( i=0; i<${#AESboot}; i=i+1 ))
do
    if [ $i -eq 0 ]; then
        end=1
    else
        end=$i
    fi
    subAES=$(printf "%c\n" ${AESboot:$i:$end})
    subHash=$(printf "%c\n" ${hashkey:$i:$end})
    Result+=$(printf "%c" $(printf "%x" $(( 0x${subAES} ^ 0x${subHash} ))))
done

echo ${Result} | perl -ne 's/([0-9a-f]{2})/print chr hex $1/gie'
