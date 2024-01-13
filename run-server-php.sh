#!/bin/sh

echo "Running local server on port 8000"
php -S localhost:8000 -t ./server-backend/php
