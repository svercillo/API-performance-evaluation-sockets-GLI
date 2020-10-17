# CloudFlare Systems Assignment

## Purpose:
This assignment uses socket programming to send requests to API endpoints getting JSON data in the form of either an a JSON array or object. A specified number of HTTP request is sent to a specified url, port and request link, and the return response is then parsed manually.  Some awesome stats are displayed on the requests sent including the average request time, max and min bytes recieved etc. and execution time.

## Project Details: 
-Written in C, implemented via socket programming to perform HTTP GET requests. The connection hangs and is I/O blocking and the program will wait until a response is sent back from requested server.
-There is a good amount of error handling done to make sure the CLI is usable

## Usage
### Build
- Run `make`

## To run
- Run ./bin/socket with the url as `https://my-worker.snvercil.workers.dev/`
- Run `systems-assignment` executable with the following flags:

## Flags
`--url`       Non null string URL that contains the port number and any additional request. e.g. 'https://my-worker.snvercil.workers.dev:80/go-to-link'
`--profile`   Positive integer for the number of desired requests to url
`--help`      Provides a usage



## Performance

##### My Cloudflare Website
- `https://my-worker.snvercil.workers.dev:80/go-to-link`

  ![](/screenshots/cloudflare.PNG)

  

##### Dummy Rest API    
- `http://dummy.restapiexample.com:80/api/v1/employee/1`

![](/screenshots/other.PNG)



## Observations on Perfomance##

Cloudflare's website performs about 6 times faster than the dummy API website! (in the limited trials I had done) This is because Cloudflare optimizes the delivery of website resources. Cloudflare’s data centers serve website’s static resources and ask the orgin web server for dynamic content. Cloudflare’s global network provides a faster route from site visitors to the data centers than would be available to a visitor directly requesting one's site. Even with the Cloudflare interface between a website and requesters, resource requests arrive to the requester much sooner.


