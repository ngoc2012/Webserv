var     output = {};

const path = "save.json";

process.stdin.on('data', function(data_in) {
    const fs = require('fs');
    var input = data_in.toString().trim();
    jason = JSON.parse(input);
    data = "";
    try {
        data = fs.readFileSync(path, 'utf8');
    } catch (error) {
        output["error"] = "Error read file";
    }
    conv_json = {};
    try 
    {
        if (data !== "")
            conv_json = JSON.parse(data);
    }
    catch (ParserErr)
    {
        //output["error"] = ParserErr;
        output["error"] = "Parse Err";
        return ;
    }
    if (jason["action"] == "signup")
    {
        if(conv_json[jason["name"]])
            output["status"] = "existed";
        else
        {
            conv_json[jason["name"]] = jason["password"]; 
            fs.writeFile(path, JSON.stringify(conv_json), 'utf8', function(err) 
            {
                output["error"] = "err";
            });
            output["status"] = "OK";
        }
    }
});

process.stdin.on('end', function() {
    console.log ("Status: 200 OK\r\n");
    console.log ("Content-Type: plain/text; charset=utf-8\r\n\r\n");
    console.log(JSON.stringify(output));
});

process.stdin.on('error', function(err) {
    output["error"] = err;
});
