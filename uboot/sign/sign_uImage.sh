#!/bin/sh

# This script sign uImage with sha1.
openssl dgst -sha1 -sign rootkey.pem -out uImage_signature uImage
cat uImage uImage_signature > uImage+signature




