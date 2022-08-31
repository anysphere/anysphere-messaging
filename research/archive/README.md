
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

## secret-key crypto

friend registration:

add-friend sualeh
outputs: send werjfoiqwjefpoqwjfpoqwjfpoqwjfpowejfpoqwjfpqwjef to sualeh
input from sualeh: _______

the werjqefjqoweijfpoqwjfpoqwjfpoqwejfpo encodes: {“public key”: “qwoiefoiqwjef”, “index”: xx}. hence it is safe to send it over an unsafe channel. also if people already have a way of communicating online then they are already being tracked so itsok. who is client/server? maybe whoever’s public key comes lexicographically first. or maybe just simply whoever’s index comes lexicographically first. that’s easier.

for non-friends tho: need to do the callign thing.

should this public key be the same for all friends? it doesn't need to be. but it also doesn't hurt if it is. so i suppose it might as well be. then later we can expand this into a PKI of some sort. one interesting idea would be to integrate with ENS.

- we would like to have forward secrecy. that is, if a user's machine is compromised at time T, no encrypted message before time T can be decrypted. however, i don't think that signal's future secrecy makes much sense, since it only protects against a very specific, small attack.
- libsodium has a secretstream API. i'm not sure if this is good for us though. for example, i think we would like it to be the case that messages can be removed, because our server does not implement a queue interface. the nice thing is that it provides ratcheting automatically, for forward secrecy.
- options:
    1. only able to send a new message if the previous message has been ACK-ed. this feels non-ideal... or maybe not.
    2. i think ACK-ing is what we want. you can send, replace messages, and when responding to a message you can ACK received messages.

the addra idea of mailboxes can exist in layers, operating on different schedules. you can decide to move someone into a more frequent layer when you have a conversation with them.

forward secrecy counterpoint: "Forward secrecy only protects against the disclosure of a private key. In almost all cases, the ability to get a private key is going to mean that you also get any saved messages. So if your chat application requires the long term storage of old messages then you can skip the forward secrecy part."

hence, forward secrecy should sort of be manual. when you say "hey pls delete this message" then a new key should be generated.

conclusion: use https://libsodium.gitbook.io/doc/secret-key_cryptography/encrypted-messages. worry about forward secrecy later.

NOTE: DIALING NEEDS KEY-PRIVATE PKE.

also note: tink would not be possible to use, i think. it is unclear how they handle nonces, and i am scared that what they have is not key-private.


# useful resources

1. wire whitepaper: https://wire-docs.wire.com/download/Wire+Security+Whitepaper.pdf
    - the wire implementation of the signal protocol, called proteus. it uses libsodium.
    - the signal protocol implementation. it uses openSSL.


# things to address in whitepaper

WE NEED A FORMAL SECURITY DEFINITION. OTHERWISE THIS WILL BE VERY VERY VERY HARD.
THE FORMAL SECURITY DEFINITION NEEDS TO INCLUDE ALL ASPECTS. IS PUNG'S DEFINITION ENOUGH,
SUPPOSING THE SECRET KEY ENCRYPTION SCHEME IS ONLY IND-CCA2 AND NOT NECESSARILY KEY-PRIVATE?
IS THIS EVEN POSSIBLE?

1. key-privacy for secret key encryption.
    - specifically with respect to nonces
2. the number of rows in the DB needs to be constant, or needs to be verified somehow. addra doesn't address this.
3. key-privacy for PKE for dialing.
4. forward secrecy, and when and why it matters.


# ideas

1. multi-level PIR kind of thing? in first level each person has a ton of entries, one for each friend, and each entry is small. in the second level every person has 1 big entry, which contains data. the first level points to an index in the second level. there may be multiple second levels: say 10 of them, and the ith one can only be accessed in rounds that are i modulo 10.