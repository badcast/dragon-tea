 #!/usr/bin/sh

xdg-open localhost:8000/api/auth.php
php -S localhost:8000 -t ./server-php
