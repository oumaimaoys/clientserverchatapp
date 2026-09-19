# Terminal Chat app in c++

## Architecture
```
+--------+    client 1
|        |  / 
| server |  -- client 2
|        |  \
+--------+    ....
```

clients connect to the same server, and choose which client to chat with and they are a one-to-one chat

