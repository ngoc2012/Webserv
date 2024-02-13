// testfile.js
var     output = {};

// Event listener for data on stdin
process.stdin.on('data', function(data) {
    // Convert the input data to string
    var input = data.toString().trim();
    
    // Process the input as needed
    //console.log("Received input:", input);
    output["input"] = input;
    
    // You can perform any other operations with the input here
});

// Event listener for the end of stdin
process.stdin.on('end', function() {
    // End of input, if needed
    //console.log("End of input.");
    console.log(JSON.stringify(output));
});

// Optionally, you can handle errors on stdin
process.stdin.on('error', function(err) {
    // Handle any errors that may occur
    //console.error("Error reading input:", err);
    output["error"] = err;
});
