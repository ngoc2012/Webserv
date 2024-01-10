#!/usr/bin/php-cgi
<?php
// Content-type header to specify that it's a PHP script
header("Content-type: text/plain");

echo "Content-type: text/plain\n\n";
echo "PHP CGI Test Script\n\n";

// Display PHP version
echo "PHP Version: " . phpversion() . "\n";

// Display PHP configuration
phpinfo();
?>

