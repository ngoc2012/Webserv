sudo cp nginx.conf /etc/nginx/conf.d
sudo systemctl reload nginx
sudo systemctl status nginx
cat /var/log/nginx/error.log