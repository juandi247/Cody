package main

import (
	"fmt"
	"net"
	"strconv"
	"sync"
	"time"
	//"time"
)

func main() {
	fmt.Println("starting goroutines")

	var wg sync.WaitGroup

	for i := 1; i < 2; i++ {
		wg.Add(1)
		go test_connection(i, &wg)
	}
	wg.Wait()
}

func test_connection(index int, wg *sync.WaitGroup) {

	conn, err := net.Dial("tcp", "localhost:8090")
	if err != nil {

		fmt.Printf("fallo algo %v", err)
		return
	}

	defer conn.Close()

	fmt.Printf("DIAl para la conex: %d done, vamos a escribir", index)

	message := " message from goroutine" + strconv.Itoa(index) + "\n"
	//	time.Sleep(time.Second * 2)
	bytesWritten, err := conn.Write([]byte(message))

	if err != nil {
		fmt.Errorf("error", err)
	}
	if bytesWritten < len(message) {

		fmt.Errorf("not sendet all the data")

	}
	time.Sleep(time.Second * 2)
	bytesWritten, err = conn.Write([]byte("Segunda parte \n"))

	fmt.Println("Written goroutine ", index)
	wg.Done()
}
