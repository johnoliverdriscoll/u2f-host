[![npm version](https://badge.fury.io/js/u2f-host.svg)](https://badge.fury.io/js/u2f-host)

libu2f-host for NodeJS
======================

Introduction
------------

This is a native addon for NodeJS that wraps [Yubico's libu2f-host](https://github.com/Yubico/libu2f-host) convenience functions.
It can be used to register and authenticate with remote services using U2F hardware tokens directly from NodeJS without a browser.

License
-------

The library is licensed under the LGPLv2+ license.

Usage
-----

```javascript
/**
 * Discover U2F tokens.
 * @return instance of U2fHost with hardware tokens discovered.
 * @throws Error description.
 */
U2fHost.discover = function() {};

/**
 * Register U2F token.
 * @param registerRequest JSON String of the request received from remote service.
 * @param origin URL String producing registerRequest.
 * @return JSON String of signature response data received from U2F token.
 * @throws Error description.
 */
U2fHost.prototype.register = function(registerRequest, origin) {};

/**
 * Authenticate U2F token.
 * @param authRequest JSON String of the request received from remote service.
 * @param origin URL String of service that generated authRequest.
 * @return JSON String of signature response data received from U2F token.
 * @throws Error description.
 */
U2fHost.prototype.authenticate = function(authRequest, origin) {};

module.exports = U2fHost;
```

### Register

```javascript
const assert = require('assert');
const devices = require('u2f-host').discover();

/* Get register request somehow */
const u2f = require('u2f');
const appId = 'https://example.com';
const registerRequest = JSON.stringify(u2f.request(appId));

/* Sign request with U2F token */
const registerResponse = JSON.parse(devices.register(registerRequest, appId));

/* Verify response somehow */
const registerStatus = u2f.checkRegistration(registerRequest, registerResponse);
assert(registerStatus.successful);

/* Store registration data for authentication later */
//require('fs').writeFileSync(JSON.stringify(registerStatus), 'registerStatus.json');
```


### Authenticate

```javascript
const assert = require('assert');
const devices = require('u2f-host').discover();

/* Load registration data for authentication */
//const registerStatus = require('./registerStatus.json');

/* Get auth request somehow */
const u2f = require('u2f');
const appId = 'https://example.com';
const authRequest = JSON.stringify(u2f.request(appId, registerStatus.keyHandle));

/* Sign request with U2F token */
const authResponse = JSON.parse(devices.authenticate(authRequest, appId));

/* Verify response somehow */
const authStatus = u2f.checkAuthentication(authRequest, authResponse);
assert(authStatus.successful);
```


Building
--------

### Dependencies

* [libu2f-host](https://github.com/Yubico/libu2f-host)


### Instructions

    $ npm i
