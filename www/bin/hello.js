// testfile.js
var     output = {};

const path = "save.json"

// Event listener for data on stdin
process.stdin.on('data', function(data) {
    // Convert the input data to string
    const fs = require('fs');

    var input = data.toString().trim();
    
    // Process the input as needed
    //console.log("Received input:", input);

    output["input"] = input;
    
    //console.log(input);
    jason = JSON.parse(input);
/*
    fs.access(path, fs.constants.F_OK, (err) => 
    {
        if (err) 
        {
            fs.writeFile(path, "{}", 'utf8', function(err) 
            {
                if (err)
                    console.log("Error writing to empty file:", err); 
                else
                    console.log("Empty file write successfully!");
            });
        } 
    });


    fs.readFile(path, 'utf8', function(err, data) 
    {
        if (err) 
        {
            console.error("Error reading file:", err);
            return;
        }

        try 
        {
            // Parse JSON data
            conv_json = JSON.parse(data);
            //console.log("Conv Json: ", conv_json);


            if (typeof conv_json === 'undefined')
                conv_json = {};

            if (jason["action"] == "signup")
            {
                if(conv_json[jason["name"]])
                    console.log("Already saved");
                else
                {
                    conv_json[jason["name"]] = jason["password"]; 
                    fs.writeFile(path, JSON.stringify(conv_json), 'utf8', function(err) 
                    {
                        if (err)
                            console.log("Error writing to file ---- (signup)"); 
                        else
                            console.log("Data saved to save.json successfully! ---- (signup)");
                    });
                }
            }
            else if (jason["action"] == "signout") 
            {
                if(conv_json[jason["name"]])
                {
                    delete conv_json[jason["name"]];
                    fs.writeFile(path, JSON.stringify(conv_json), 'utf8', function(err) 
                    {
                        if (err)
                            console.log("Error writing to file ---- (signout)"); 
                        else
                            console.log("Data saved to save.json successfully! ---- (signout)");
                    });
                }
                else
                {
                    console.log("User doesn't no exist");
                }
            }
        }
        catch (parseError )
        {
            console.log("Error parsing JSON");
        }
    });
*/
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
