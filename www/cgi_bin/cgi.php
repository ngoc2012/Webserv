<?php

// Check if the correct number of arguments is provided
if ($argc != 2) {
    echo "Usage: php execute.php <filename.php>\n";
    exit(1);
}

// Get the filename from the command-line arguments
$filename = $argv[1];

// Check if the file exists
if (!file_exists($filename)) {
    echo "Error: File '$filename' not found.\n";
    exit(1);
}

// Check if the file has a .php extension
if (pathinfo($filename, PATHINFO_EXTENSION) !== 'php') {
    echo "Error: Only PHP files are allowed.\n";
    exit(1);
}

// Execute the PHP file
include $filename;

?>

