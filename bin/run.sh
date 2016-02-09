result=${PWD}
result2=${PWD#/} 
echo $result
echo $result2
if [ -t 1 ]; then
    echo $TERM
    $./start.sh
    arg=$(cat ./test)
    nohup $arg &
else
    # stdout is a tty
    xterm ./start.sh
    arg=$(cat ./test)
    nohup $arg &
fi

