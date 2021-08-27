# Latency Send

Send a number of messages (RUNS) with a defined message size (SIZE) to ```latency_rec``` and evaluates the average latency for a single message transport. 
You can switch on zero copy mode by setting the ```-z``` option. The number of used publisher can be set by ```-b```.

## Syntax

```latency_snd -r RUNS -d DELAY -b MEMBUFFER -z -s SIZE```
