# TU-2016WS-BS-exercise-1b-mastermind
an exercise for Betriebsysteme WS2016 

## What it does
mastermind is a tcp/ip client/server game written in c

## SYNOPSIS
*server <server-port> <secret-sequence>*
Example: server 1280 wwrgb

client <server-hostname> <server-port>
Example: client localhost 1280

## OPTIONS / FLAGS
* <server-port>: Port where the server is listen to
* <secret-sequence>: A sequence of following characters which represent colors (**b**eige, **d**unkelblau, **g**rün, **o**range, **r**ot, **s**chwarz, **v**iolett, **w**eiß)
