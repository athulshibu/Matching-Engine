
# Order Matching Engine

This project creates an Order Matching Engine that uses TCP/IP protocols to accept orders on one thread and process them on another. Processing orders involves matching a Buy order with a suitable Sell order, even if the required quantity cannot be filled completely (partial fills).

## Understanding Order Structure
There are two types of orders; Buy represented by an *id* value of 1, and Sell represented by 0. Each order also has a Price at which the order can be fulfilled (bought or sold) and the quantity of shares to be sold. For simplicity's sake, it is assumed that this matching engine only deals in a single stock. The order also stores the time stamp at which the order was entered into the system (not the time stamp of creation).

Each Buy or Sell order has a unique ID, starting from 1 to how many ever orders are created. Buy and Sell orders may have the same IDs, So to identify an individual, unique order, the ID and Type are both necessary.

## Understanding a Trade order
A trade order is a structure that stores the ID of buying order, the ID of the selling order, quantity price of sale, and quantity. A time stamp keeps track of the sale time. A trade can be triggered in two ways which will be explored later.

## Creating an order
Quite a few assumptions have been made due to time considerations. As mentioned earlier, it is assumed that only one stock is used, so there is no necessity for a unique stock identifier. It is assumed that the price can only range from 100 to 102, with a tick of 0.25, i.e., there are only 9 possible prices. The quantities of orders are constrained to only allow for transactions as multiples of 25, with a maximum of 100 shares at a time.

The client creates a 4-character string, with 3 digits followed by a '\n' to signal the end of the message. The first digit is either 1 or 0, representing Buy or Sell. The second digit represents the price and could have a random value from 0 to 9. This is converted to price using the formula `100 + n*0.25`. The third digit represents the quantity, which can only be one of four values, hence is represented by a random digit from 1 to 4. It is converted to the quantity using the formula `25*n`.

## Order Matching
There are two ways to match orders; Either the buying price is exactly equal to the selling price, or the buying price is greater than or equal to the selling price. Both have been implemented and can be switched from one mode to another by commenting at the right places.

### Order matching for Buying price equal to the selling price
If a buy order and a sell order exist where the price is equal, the lower of the two quantities is traded. The trade order stores the Buying order ID, selling order ID, price of trading, quantity traded, and the time stamp at the time of trading. 

In order to run this program, uncomment lines 77-88 in *matching_engine.cpp* and comment lines 90-103.

### Order matching for Buying price greater than or equal to the Selling price
In an order matching system, the buyers and seller, who place the order, only know their hard limits. They are working with imperfect information, thus leaving money on the table. This leaves room for the matching engine to make money on their trades. The alternative is to let either the buyer or the seller make a profit off of their trades. The matching engine does not try to maximize profits, instead tries to satisfy as many orders as possible, prioritizing orders on a First-In-First-Out model.

When a buy order is set for a price of, say 102, and a sell order is set for 100.5, there is a difference of 1.5 per stock that can be acquired by the matching engine. The *matchingEngineProfit* variable tracks all possible such profits, adding every such difference for every stock sold.

In order to run this program, comment lines 77-88 in *matching_engine.cpp* and uncomment lines 90-103.


## Sending created Orders from client to server

TCP/IP protocols were integrated into the program using the *winsock2.h* library. The *matching_engine.cpp* acts as the server socket, receiving orders in the above-mentioned format from the client socket in *order_generator.cpp*. Only a single socket is set to send data from the client side and receive from the server side. This could potentially cause data loss due to desync, but this is limited by appropriately placed delays. While testing, it is better to set the delay value in line 76 of *order_generator.cpp* to a higher value. This would eliminate data drops for the most part, as it is assumed for this assignment that the data travels through a perfect system.

The sender and receiver addresses are both local hosts, i.e., "127.0.0.1", and the port is set to 80. The server socket has to be initialized first, and the client socket only activates while the server socket is in listening mode. Hence *matching_engine.cpp* has to be running first, followed by *order_generator.cpp*. The *setsockopt()* sets the port to Reusabe mode, without which the socket does not bind (I spent an entire day solving only this issue).

## Matching Engine
Once the socket is set to listening mode, the engine is ready to accept messages. The server side receives a string with 4 characters, and these characters are then converted into readable data using the processing mentioned above. This is then used to create an order structure and sent to the matching engine. This is done by creating a thread for the function, then setting all receivers to work inside this function. It acts as the gateway for messages into the matching engine. 

Multi-threading is necessary for this because the socket has to keep receiving data while the data is processed and added into the same structure used for processing. Received messages are converted into orders and stored in the same structure that provides data from matching. Data is both added and modified, sometimes deleted, parallelly, thus necessitating multi-threading.

## Experimental Results

The order matching engine is able to process/match over to 3800 orders per minute, and there is no upper limit to the volume of data that can be processed, subject to the limits of the server. 

### Running the code
I used Visual Studio Code to write and edit the programs and ran the code using CMD (Windows 11). Usage of Windows makes it necessary to use *winsock2.h* and *windows.h* to use TCP/IP protocols and Multi-threading functionality, which could've been easily achieved in Linux using *socket* and *threads*. C++11 was installed using MinGW, as was winsock2. 

STEP 1: To run the program, first open CMD and navigate to the location of the files using the *cd* command. 

STEP 2: Run the following code `g++ matching_engine.cpp -o server.exe -lws2_32 && server.exe` to activate the server socket.

STEP 3: Run the following code `g++ order_generator.cpp -o client.exe -lws2_32 && client.exe` to activate the client side and send data.
