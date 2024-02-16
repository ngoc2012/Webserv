# Generate 10,000 bytes of random data
#random_data=$(openssl rand -base64 10000)
random_data=$(dd if=/dev/urandom bs=10000 count=1 status=none | base64 | tr -d '\n' | head -c 10000)

## Set the chunk size
#chunk_size=1000
#
## Initialize variable to store concatenated chunks
#concatenated_data=""
#
## Loop through the data in chunks and concatenate
#for ((i=0; i<${#random_data}; i+=chunk_size)); do
#    chunk="${random_data:$i:$chunk_size}"
#    concatenated_data="${concatenated_data}${chunk}"
#done
#
## Use curl to send the concatenated data in one POST request with Transfer-Encoding: chunked header
#curl -X POST -H "Transfer-Encoding: chunked" --data-binary "$concatenated_data" http://127.0.2.1:4242/directory/youpi.bla
#
## Generate 10,000 bytes of random data
#random_data=$(openssl rand -base64 10000)

# Set the desired chunk size
chunk_size=1000

# Loop through the data in chunks and concatenate
for ((i=0; i<${#random_data}; i+=chunk_size)); do
    chunk="${random_data:$i:$chunk_size}"
    concatenated_data="${concatenated_data}${chunk}"
done

# Use curl to send the concatenated data in one POST request with Transfer-Encoding: chunked header
curl -X POST -H "Transfer-Encoding: chunked" --data-binary @<(echo -n "$concatenated_data") http://127.0.2.1:4242/directory/youpi.bla

