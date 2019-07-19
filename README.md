# Concurrent Programming

## directory

- TCP (Network Program (socket by TCP))
  - server.c
  - client.c
- proccess
  - prod2cons.c (Producer Consumer Problem)
- Thread
  - determinant.c ()
  - prod2cons.c (Producer Consumer Problem)

## TCP

### Compile
`> make`

### Server-side run
Lock file version <br>
`> ./server-lock <file-name>` <br>
No lock file version <br>
`> ./server-nolock <file-name>`
> file-name : Specify the file name to log

### Client-side run
`> ./client <server-name> <client-name> <repeat-count>`
> server-name : Server-side's name or IP <br>
> client-name : A name that can identify the client that is running this program <br>
> repeat-count : Number of times to repeat message sending and receiving

## proccess

### Compile
`> make`

### Run
`> ./prod2cons` <br>
Can confirm parallel access of producer process and consumer process of ring buffer which is shared memory. <br>

Do the time taken from process creation as the final output as follows. <br>
`<Real time>,<CPU time>` 

## thread
