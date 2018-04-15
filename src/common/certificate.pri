
# Name of the application signing certificate
APPCERT = "\"Developer ID Application: Three Monkeys International Inc. (42EJ97Y7M3)\""

# Cert OU
CERT_OU = "\"42EJ97Y7M3\""

# Sha1 of the siging certificate
CERTSHA1 = 25EFCD6194962D6C1360779C1B09CA89B85870D8

DEFINES += kSigningCertCommonName=\\\"$${APPCERT}\\\"
