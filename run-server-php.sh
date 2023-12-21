 #!/usr/bin/sh

php -S localhost:8000 -t ./server-backend/php
xdg-open localhost:8000/api/auth.php
