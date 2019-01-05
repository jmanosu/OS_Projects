CC=gcc

clients: otp_dec.c otp_enc.c
	$(CC) -o otp_dec otp_dec.c
	$(CC) -o otp_enc otp_enc.c

servers: otp_dec_d.c otp_enc_d.c
	$(CC) -o otp_dec_d otp_dec_d.c
	$(CC) -o otp_enc_d otp_enc_d.c

keygen: keygen.c
	$(CC) -o keygen keygen.c

all: keygen.c otp_dec_d.c otp_enc_d.c otp_dec.c otp_enc.c
	$(CC) -o keygen keygen.c
	$(CC) -o otp_dec_d otp_dec_d.c
	$(CC) -o otp_enc_d otp_enc_d.c
	$(CC) -o otp_dec otp_dec.c
	$(CC) -o otp_enc otp_enc.c

clean:
	rm otp_dec otp_enc otp_dec_d otp_enc_d keygen
