# VerneTask
For building project go in VerneTask directory and write ```$ make```. Which should build and create two binarys in bin folder.

## Server
To run server write: \
```$ bin/server <PORT>```

## Client
To run clinet write: \
```$ bin/client```

### Commands
To use client we have 5 commands. 
- For connecting and disconnecting: \
```> CONNECT <PORT> <NAME>``` \
```> DISCONNECT``` 

- For subscribing and unsubscribing: \
```> SUBSCRIBE <TOPIC>``` \
```> UNSUBSCRIBING <TOPIC>``` 

- For publishing data: \
```> PUBLISH <TOPIC> <DATA>```