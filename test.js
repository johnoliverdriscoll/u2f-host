'use strict';
const u2fh = require('.');
const u2f = require('u2f');
const {expect, should} = require('chai');
should();

describe('u2fh', function() {

  const appId = 'https://example.com';
  let devices;
  let registerStatus;
  let authStatus;

  it('discovers', function() {
    expect(() => devices = u2fh.discover()).to.not.throw();
    devices.should.be.instanceOf(u2fh.constructor);
  });

  it('registers', function() {
    if (!devices) {
      this.skip();
    }
    this.timeout(0);
    let response;
    const request = u2f.request(appId);
    expect(() => response = devices.register(JSON.stringify(request), appId)).to.not.throw();
    expect(() => registerStatus = u2f.checkRegistration(request, JSON.parse(response))).to.not.throw();
    registerStatus.successful.should.equal(true);
  });

  it('authenticates', function() {
    if (!registerStatus) {
      this.skip();
    }
    this.timeout(0);
    let response;
    const request = u2f.request(appId, registerStatus.keyHandle);
    expect(() => response = devices.authenticate(JSON.stringify(request), appId)).to.not.throw();
    expect(() => authStatus = u2f.checkSignature(request, JSON.parse(response), registerStatus.publicKey)).to.not.throw();
    authStatus.successful.should.equal(true);
  });

});
