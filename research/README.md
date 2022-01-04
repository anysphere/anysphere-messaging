
## mailbox system

- everyone gets assigned an allocation to which they write
- inspired by Addra

### security problems

- the client needs to know how many entries there are in the database to be able to form a PIR query.
- now consider the following attack:
    - the server lets person A and B get allocation indices 1 and 2, and tells them there are 10^6 db rows.
    - everyone else gets allocation index > 10^6, and the server tells them there are 2 * 10^6 db rows.
    - then, it is obvious to everyone that A and B are communicating with each other, which is BAD.

is this a problem in Addra as well? i think so, at least implicitly.

fix:
- the client should not trust the server on `db_rows`. instead, it should have a hardcoded value, say 10^6 always.
- the client should immediately complain if it gets an allocation index > 10^6.
    - this has the disadvantage that performance will be bad even with few users. on the other hand, scaling will be linear instead of quadratic which is good.