/*
    CHALLENGE 12 (text processor).
    For a text processor, can you use a doubly linked list to store text?
    The idea is to represent a “blob” of text through a struct that contains a string (for the text) and pointers to preceding and following blobs.
    Can you build a function that splits a text blob in two at a given point?
    One that joins two consecutive text blobs?
    One that runs through the entire text and puts it in the form of one blob per line?
    Can you create a function that prints the entire text or prints until the text is cut off due to the screen size?
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct textBlob textBlob;
struct textBlob{
    textBlob* previous;
    textBlob* next;
    char message[];
};

textBlob* latestBlob = 0;

//Initializes a textBlob, adding it as the next blob to the last created blob and making it the next latestBlob.
textBlob* textBlob_create(textBlob*, const char[]);

//Replaces the given string with "message <num>" and return's the strings address.
char* makeMessageWithNum(char[], size_t);

//Wrapper for textBlob_create, makes the blob contain "message <num>".
textBlob* createBlobWithNumMessage(textBlob*, const size_t);

//Splits the blob into two seperate blobs at the given index. Returns zero if index outside message or if any other error occured.
textBlob* textBlob_split(textBlob*, const size_t);

//Change target's neighbour's neighbour's to rightNeighboursBlob and leftNeighboursBlob. If there's nothing to the right of target, change latestBlob to newLatest.
void joinNeighbours(textBlob*, textBlob*, textBlob*, textBlob*);

//Print the entire text from startingBlob to the end.
void printTextblobs(textBlob* startingBlob);

//Print the entire text from startingBlob to the end, with a new line after each blob.
void printTextblobsNewline(textBlob* startingBlob);

textBlob* textBlob_create(textBlob* blob, const char message[])
{
    if(!(blob = malloc(sizeof(textBlob)+(sizeof(char)*strlen(message)))))
        return 0;

    //If not first blob, change previous blob's next blob.
    if(latestBlob)
        latestBlob->next = blob;

    //Make previous blob the latestBlob, and make latestBlob current blob.
    blob->previous = latestBlob;
    latestBlob = blob;

    //Initialize next blob to zero
    blob->next = 0;
    
    strcpy(blob->message,message);
   
    return blob;
}

char* makeMessageWithNum(char buffer[], size_t num)
{
    //Make sure num won't overflow the buffer;
    num = num > 999 ? 999 : num;

    if (0 > sprintf(buffer, "message %d\0", num))
    {
        fputs("Problem using sprintf", stderr);
        exit(EXIT_FAILURE);
    }

    return buffer;
}


textBlob* createBlobWithNumMessage(textBlob* blob, const size_t num)
{
        char buffer[13];

        if(!(blob = textBlob_create(blob, makeMessageWithNum(buffer, num))))
        {
            fputs("Problem using textBlob_create", stderr);
            exit(EXIT_FAILURE);    
        }

    return blob;
}

textBlob* textBlob_split(textBlob* blob, const size_t index)
{
    if(!blob)
    {
        return 0;
    }

    const size_t msgLen = strlen(blob->message);
    const size_t leftMsgLen = index + 1; //+1's for null termination.
    const size_t rightMsgLen = msgLen - index + 1;
    
    char* buffer = blob->message;

    char leftMessage[leftMsgLen];
    char rightMessage[rightMsgLen];

    //Return zero if caller wants to split after the string has ended.
    if(index > msgLen){
        return 0;
    }

    //Go through string and assign each char to either left or right message depending on if it went through the index or not.
    while(*buffer){
        size_t i = buffer - blob->message;
        if(i < index){
            leftMessage[i] = *buffer;
        }
        else{
            rightMessage[i - index] = *buffer;
        }
        buffer++;
    }

    //Make sure both strings are zero terminated.
    leftMessage[leftMsgLen-1] = 0;
    rightMessage[rightMsgLen-1] = 0;

    textBlob* rightBlob;
    if(!(rightBlob = malloc(sizeof(textBlob)+(sizeof(char)*rightMsgLen))))
        return 0;
    
    strcpy(rightBlob->message, rightMessage);
    rightBlob->next = blob->next;

    //If there is nothing to the right of the right blob, that means it is the new latestBlob.
    if(!rightBlob->next)
    {
        latestBlob = rightBlob;
    }else //If there is, then change the right blob's previous blob to the right blob. 
    {
        rightBlob->next->previous = rightBlob;
    }


    textBlob* leftBlob;
    if(!(leftBlob = malloc(sizeof(textBlob)+(sizeof(char)*leftMsgLen))))
        return 0;


    strcpy(leftBlob->message, leftMessage);
    leftBlob->previous = blob->previous;
    if(leftBlob->previous)//If previous blob isn't zero then change that blob's next blob to left blob.
    {
        leftBlob->previous->next = leftBlob;
    }

    //Connect left and right blob together.
    leftBlob->next = rightBlob;
    rightBlob->previous = leftBlob;

    free(blob);
    return leftBlob;
}

/*
    If the messages are laid out like this:
    (0)<->(1)<->(2)<->(3)<->(4)<->(5)
    And we join 1 and 4 together then they become this:
    (0)<->(1+4)<->(2)<->(3)<->(5)
*/
textBlob* textBlob_join(textBlob* blobOne, textBlob* blobTwo)
{
    if(!blobOne || !blobTwo){
        return 0;
    }
    size_t joinedMsgLen = strlen(blobOne->message)+strlen(blobTwo->message) + 1;//+1 for null terminator.

    textBlob* newBlob;
    if(!(newBlob = malloc(sizeof(textBlob)+(sizeof(char)*joinedMsgLen))))
        return 0;

    //Create the message for the blob.    
    strcpy(newBlob->message, blobOne->message);
    strcat(newBlob->message, blobTwo->message);

    //Join blob two's neighbours together as if blobTwo never existed.
    joinNeighbours(blobTwo, blobTwo->previous, blobTwo->next, blobTwo->previous);

    //Make blobOne's neighbours (which could've been changed by blobTwo just before) into newBlob's neighbours.
    newBlob->previous = blobOne->previous;
    newBlob->next = blobOne->next;

    //Make newBlob's neighbour's change their old neighbour (blobOne) into newBlob.
    joinNeighbours(newBlob, newBlob, newBlob, newBlob);

    free(blobOne);
    free(blobTwo);
    
    return newBlob;
}

void joinNeighbours(textBlob* target, textBlob* rightNeighboursBlob, textBlob* leftNeighboursBlob, textBlob* newLatest)
{
    //If target's right neighbour.
    if(target->next)
    {
        //Change targets right neighbour's left neighbour to new right neighbour.
        target->next->previous = rightNeighboursBlob;
    }
    else
    {
        //If there's nothing to the right change latestBlob.
        latestBlob = newLatest;
    }
    if(target->previous)
    {
        //Change targets left neighbour's right neighbour to new left neighbour.
        target->previous->next = leftNeighboursBlob;
    }
}

void printTextblobsNewline(textBlob* startingBlob)
{
    while(startingBlob){
        printf("%s\n",startingBlob->message);
        startingBlob = startingBlob->next;
    };
}

void printTextblobs(textBlob* startingBlob)
{
    while(startingBlob){
        printf("%s",startingBlob->message);
        startingBlob = startingBlob->next;
    };
}

int main()
{
    const size_t amount = 20;
    textBlob *blob, *firstBlob;

    //Make the blobs.
    for(size_t i = 0; i < amount; i++)
    {
        blob = createBlobWithNumMessage(blob, i);
        if(i == 0){firstBlob = blob;}
    }

    //Join some blobs every 4th blob.
    int i = 0;
    while(blob){
        if(i++ % 4 == 1 && blob && blob->previous && blob->previous->previous){
            blob = textBlob_join(blob, blob->previous->previous);
        }
        blob = blob->previous;
    };

    //Print the blobs.
    printTextblobs(firstBlob);
    printf("\n\n\n");
    printTextblobsNewline(firstBlob);

}