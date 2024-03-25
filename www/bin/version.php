#!/usr/bin/php-cgi
<?php
// Content-type header to specify that it's a PHP script
echo "Status: 200 OK\r\n";
echo "Content-Type: plain/text; charset=utf-8\r\n\r\n";

echo "Content-type: text/plain\n\n";
echo "PHP CGI Test Script\n\n";

// Display PHP version
echo "PHP Version: " . phpversion() . "\n";

// Display PHP configuration
phpinfo();
?>

