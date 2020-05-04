struct wav{
    char ChunkID[4];
    unsigned int ChunkSize;
    char Format[4];
    char Subchunk1ID[4];
    unsigned int Subchunk1Size;
    unsigned short int AudioFormat;
    unsigned short int NumChannels;
    unsigned int SampleRate;
    unsigned int ByteRate;
    unsigned short int BlockAlign;
    unsigned short int BitsPerSample;
    char SubChunk2ID[4];
    unsigned int Subchunk2Size;
};
