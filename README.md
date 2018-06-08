pblock-buse
===========

Utility to attach pblock connection as a block device.

Under the hood it uses [nbd](https://github.com/NetworkBlockDevice/nbd) kernel interface and [BUSE](https://github.com/acozzette/BUSE) to communicate with it.

Transactional access
--------------------

I didn't found any information about `nbd` being transactional, so currently every read or write on `nbd` is wrapped in separate `pblock` transaction.

Tests
-----

Dependencies:
 * [pgspwan](https://github.com/SupraSummus/pgspawn)
 * [pblock utilities](https://github.com/SupraSummus/pblock)

Currently tests require root previlages. Test runner uses `sudo` to provide them.

To run tests:

    make test

