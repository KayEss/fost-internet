# TLS

The `fostlib::network_connection` class supports the use of TLS (SSL) through ASIO's support of OpenSSL.

One important aspect of the TLS handshake is verifying the correctness of the server's certificate. If you're using the class directly you can opt into this through the use of the `start_ssl` method by passing it a hostname.

Server certificate checking cannot be done without knowing the expected hostname of the server at the time the encrypted handshake is performed. Due to this, of `start_ssl` is not passed a hostname then no server verification is attempted.


## Configuration options


### `fostlib::c_tls_use_standard_verify_paths` -- boolean

Turns on or off the search for OpenSSL root certificates in the standard system paths.

```ini
[TLS]
Use standard verify paths=false
```

For Android and Apple this is off by default, other platforms have it on. If you wish to only use certificates that you specify you should set this to `false`.


### `fostlib::c_extra_ca_cert_paths` -- List of strings

Sets paths to search for OpenSSL root certificates.

```ini
[TLS]
Extra CA certificate paths=["/path/to/store1", "path/to/store2"]
```

On Android this is set to the path `/system/etc/security/cacerts/` which has been in use since Ice Cream Sandwich. For all other platforms this defaults to an empty list.


### `fostlib::c_extra_ca_certificates` -- List of strings in PEM format

Extra certificates that will be accepted.

```ini
[TLS]
Extra CA certificates = ["-----BEGIN CERTIFICATE-----\n{base 64 data}\n-----END CERTIFICATE-----\n]
```

On Apple this includes one Digicert certificate and the Let's Encrypt root. This is unlikely to be enough for most applications, but is enough for all of the tests. On other platforms the list is empty because a full set of root certificates are already available.
