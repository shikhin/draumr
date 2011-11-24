
Source/System/Boot/DBAL/Src/OutputMod/Enhance/Dither.o:     file format elf32-i386


Disassembly of section .text:

00000000 <DitherInit>:
   0:	f3 c3                	repz ret 
   2:	8d b4 26 00 00 00 00 	lea    esi,[esi+eiz*1+0x0]
   9:	8d bc 27 00 00 00 00 	lea    edi,[edi+eiz*1+0x0]

00000010 <Dither>:
  10:	55                   	push   ebp
  11:	57                   	push   edi
  12:	56                   	push   esi
  13:	53                   	push   ebx
  14:	83 ec 4c             	sub    esp,0x4c
  17:	a1 42 00 00 00       	mov    eax,ds:0x42
  1c:	83 f8 18             	cmp    eax,0x18
  1f:	0f 84 54 03 00 00    	je     379 <Dither+0x369>
  25:	a1 42 00 00 00       	mov    eax,ds:0x42
  2a:	83 f8 04             	cmp    eax,0x4
  2d:	74 0a                	je     39 <Dither+0x29>
  2f:	a1 42 00 00 00       	mov    eax,ds:0x42
  34:	83 f8 08             	cmp    eax,0x8
  37:	74 08                	je     41 <Dither+0x31>
  39:	83 c4 4c             	add    esp,0x4c
  3c:	5b                   	pop    ebx
  3d:	5e                   	pop    esi
  3e:	5f                   	pop    edi
  3f:	5d                   	pop    ebp
  40:	c3                   	ret    
  41:	a1 3a 00 00 00       	mov    eax,ds:0x3a
  46:	31 f6                	xor    esi,esi
  48:	8b 15 3e 00 00 00    	mov    edx,DWORD PTR ds:0x3e
  4e:	8d 04 40             	lea    eax,[eax+eax*2]
  51:	89 44 24 3c          	mov    DWORD PTR [esp+0x3c],eax
  55:	31 c0                	xor    eax,eax
  57:	83 fa 01             	cmp    edx,0x1
  5a:	0f 84 b7 02 00 00    	je     317 <Dither+0x307>
  60:	c7 44 24 38 00 00 00 	mov    DWORD PTR [esp+0x38],0x0
  67:	00 
  68:	ba ff 00 00 00       	mov    edx,0xff
  6d:	8d 76 00             	lea    esi,[esi+0x0]
  70:	8b 5c 24 60          	mov    ebx,DWORD PTR [esp+0x60]
  74:	8b 6c 24 60          	mov    ebp,DWORD PTR [esp+0x60]
  78:	01 c3                	add    ebx,eax
  7a:	0f b6 4c 05 02       	movzx  ecx,BYTE PTR [ebp+eax*1+0x2]
  7f:	0f b6 3b             	movzx  edi,BYTE PTR [ebx]
  82:	83 e1 e0             	and    ecx,0xffffffe0
  85:	c1 ff 06             	sar    edi,0x6
  88:	09 f9                	or     ecx,edi
  8a:	0f b6 7c 05 01       	movzx  edi,BYTE PTR [ebp+eax*1+0x1]
  8f:	8d 68 03             	lea    ebp,[eax+0x3]
  92:	89 6c 24 34          	mov    DWORD PTR [esp+0x34],ebp
  96:	83 e7 e0             	and    edi,0xffffffe0
  99:	c1 ff 03             	sar    edi,0x3
  9c:	09 f9                	or     ecx,edi
  9e:	8b 7c 24 64          	mov    edi,DWORD PTR [esp+0x64]
  a2:	88 0c 37             	mov    BYTE PTR [edi+esi*1],cl
  a5:	8b 0d 3a 00 00 00    	mov    ecx,DWORD PTR ds:0x3a
  ab:	83 c6 01             	add    esi,0x1
  ae:	89 74 24 24          	mov    DWORD PTR [esp+0x24],esi
  b2:	83 e9 01             	sub    ecx,0x1
  b5:	83 f9 01             	cmp    ecx,0x1
  b8:	0f 86 b0 02 00 00    	jbe    36e <Dither+0x35e>
  be:	03 44 24 3c          	add    eax,DWORD PTR [esp+0x3c]
  c2:	31 f6                	xor    esi,esi
  c4:	03 44 24 60          	add    eax,DWORD PTR [esp+0x60]
  c8:	31 ff                	xor    edi,edi
  ca:	89 5c 24 20          	mov    DWORD PTR [esp+0x20],ebx
  ce:	31 ed                	xor    ebp,ebp
  d0:	c7 44 24 28 01 00 00 	mov    DWORD PTR [esp+0x28],0x1
  d7:	00 
  d8:	8b 5c 24 20          	mov    ebx,DWORD PTR [esp+0x20]
  dc:	0f b6 4b 03          	movzx  ecx,BYTE PTR [ebx+0x3]
  e0:	01 ce                	add    esi,ecx
  e2:	0f b6 4b 04          	movzx  ecx,BYTE PTR [ebx+0x4]
  e6:	01 cf                	add    edi,ecx
  e8:	0f b6 4b 05          	movzx  ecx,BYTE PTR [ebx+0x5]
  ec:	01 cd                	add    ebp,ecx
  ee:	66 81 fe ff 00       	cmp    si,0xff
  f3:	89 d1                	mov    ecx,edx
  f5:	0f 46 ce             	cmovbe ecx,esi
  f8:	66 81 ff ff 00       	cmp    di,0xff
  fd:	8b 74 24 64          	mov    esi,DWORD PTR [esp+0x64]
 101:	0f 47 fa             	cmova  edi,edx
 104:	66 81 fd ff 00       	cmp    bp,0xff
 109:	0f 47 ea             	cmova  ebp,edx
 10c:	89 cb                	mov    ebx,ecx
 10e:	66 89 4c 24 1c       	mov    WORD PTR [esp+0x1c],cx
 113:	89 e9                	mov    ecx,ebp
 115:	83 e5 1f             	and    ebp,0x1f
 118:	c1 eb 06             	shr    ebx,0x6
 11b:	83 e1 e0             	and    ecx,0xffffffe0
 11e:	09 d9                	or     ecx,ebx
 120:	89 fb                	mov    ebx,edi
 122:	81 e3 e0 ff 00 00    	and    ebx,0xffe0
 128:	83 e7 1f             	and    edi,0x1f
 12b:	c1 fb 03             	sar    ebx,0x3
 12e:	09 d9                	or     ecx,ebx
 130:	8b 5c 24 24          	mov    ebx,DWORD PTR [esp+0x24]
 134:	66 89 7c 24 30       	mov    WORD PTR [esp+0x30],di
 139:	0f b7 7c 24 1c       	movzx  edi,WORD PTR [esp+0x1c]
 13e:	66 89 6c 24 2e       	mov    WORD PTR [esp+0x2e],bp
 143:	0f b7 6c 24 30       	movzx  ebp,WORD PTR [esp+0x30]
 148:	88 0c 1e             	mov    BYTE PTR [esi+ebx*1],cl
 14b:	0f b6 18             	movzx  ebx,BYTE PTR [eax]
 14e:	83 e7 3f             	and    edi,0x3f
 151:	0f b6 70 01          	movzx  esi,BYTE PTR [eax+0x1]
 155:	66 89 7c 24 32       	mov    WORD PTR [esp+0x32],di
 15a:	0f b7 ff             	movzx  edi,di
 15d:	8d 0c 7f             	lea    ecx,[edi+edi*2]
 160:	c1 f9 04             	sar    ecx,0x4
 163:	01 d9                	add    ecx,ebx
 165:	66 81 f9 ff 00       	cmp    cx,0xff
 16a:	0f 47 ca             	cmova  ecx,edx
 16d:	88 08                	mov    BYTE PTR [eax],cl
 16f:	8d 4c 6d 00          	lea    ecx,[ebp+ebp*2+0x0]
 173:	c1 f9 04             	sar    ecx,0x4
 176:	01 f1                	add    ecx,esi
 178:	66 81 f9 ff 00       	cmp    cx,0xff
 17d:	0f 47 ca             	cmova  ecx,edx
 180:	88 48 01             	mov    BYTE PTR [eax+0x1],cl
 183:	0f b7 4c 24 2e       	movzx  ecx,WORD PTR [esp+0x2e]
 188:	89 6c 24 1c          	mov    DWORD PTR [esp+0x1c],ebp
 18c:	0f b6 68 02          	movzx  ebp,BYTE PTR [eax+0x2]
 190:	8d 34 49             	lea    esi,[ecx+ecx*2]
 193:	c1 fe 04             	sar    esi,0x4
 196:	01 ee                	add    esi,ebp
 198:	0f b6 68 03          	movzx  ebp,BYTE PTR [eax+0x3]
 19c:	66 81 fe ff 00       	cmp    si,0xff
 1a1:	0f 47 f2             	cmova  esi,edx
 1a4:	89 f3                	mov    ebx,esi
 1a6:	8d 34 bf             	lea    esi,[edi+edi*4]
 1a9:	c1 fe 04             	sar    esi,0x4
 1ac:	01 ee                	add    esi,ebp
 1ae:	8b 6c 24 1c          	mov    ebp,DWORD PTR [esp+0x1c]
 1b2:	66 81 fe ff 00       	cmp    si,0xff
 1b7:	0f 47 f2             	cmova  esi,edx
 1ba:	88 58 02             	mov    BYTE PTR [eax+0x2],bl
 1bd:	89 f3                	mov    ebx,esi
 1bf:	8d 74 ad 00          	lea    esi,[ebp+ebp*4+0x0]
 1c3:	0f b6 68 04          	movzx  ebp,BYTE PTR [eax+0x4]
 1c7:	c1 fe 04             	sar    esi,0x4
 1ca:	88 58 03             	mov    BYTE PTR [eax+0x3],bl
 1cd:	01 ee                	add    esi,ebp
 1cf:	0f b6 68 05          	movzx  ebp,BYTE PTR [eax+0x5]
 1d3:	66 81 fe ff 00       	cmp    si,0xff
 1d8:	0f 47 f2             	cmova  esi,edx
 1db:	89 f3                	mov    ebx,esi
 1dd:	8d 34 89             	lea    esi,[ecx+ecx*4]
 1e0:	c1 fe 04             	sar    esi,0x4
 1e3:	01 ee                	add    esi,ebp
 1e5:	0f b6 68 06          	movzx  ebp,BYTE PTR [eax+0x6]
 1e9:	66 81 fe ff 00       	cmp    si,0xff
 1ee:	0f 47 f2             	cmova  esi,edx
 1f1:	88 58 04             	mov    BYTE PTR [eax+0x4],bl
 1f4:	89 f3                	mov    ebx,esi
 1f6:	0f b7 74 24 32       	movzx  esi,WORD PTR [esp+0x32]
 1fb:	88 58 05             	mov    BYTE PTR [eax+0x5],bl
 1fe:	66 c1 ee 04          	shr    si,0x4
 202:	01 ee                	add    esi,ebp
 204:	0f b7 6c 24 30       	movzx  ebp,WORD PTR [esp+0x30]
 209:	66 81 fe ff 00       	cmp    si,0xff
 20e:	0f 47 f2             	cmova  esi,edx
 211:	89 f3                	mov    ebx,esi
 213:	0f b6 70 07          	movzx  esi,BYTE PTR [eax+0x7]
 217:	66 c1 ed 04          	shr    bp,0x4
 21b:	88 58 06             	mov    BYTE PTR [eax+0x6],bl
 21e:	01 ee                	add    esi,ebp
 220:	bd ff ff ff ff       	mov    ebp,0xffffffff
 225:	66 81 fe 00 01       	cmp    si,0x100
 22a:	0f 44 f5             	cmove  esi,ebp
 22d:	89 f3                	mov    ebx,esi
 22f:	88 58 07             	mov    BYTE PTR [eax+0x7],bl
 232:	0f b7 6c 24 2e       	movzx  ebp,WORD PTR [esp+0x2e]
 237:	0f b6 70 08          	movzx  esi,BYTE PTR [eax+0x8]
 23b:	66 c1 ed 04          	shr    bp,0x4
 23f:	01 ee                	add    esi,ebp
 241:	bd ff ff ff ff       	mov    ebp,0xffffffff
 246:	66 81 fe 00 01       	cmp    si,0x100
 24b:	0f 44 f5             	cmove  esi,ebp
 24e:	89 f3                	mov    ebx,esi
 250:	8d 34 fd 00 00 00 00 	lea    esi,[edi*8+0x0]
 257:	29 fe                	sub    esi,edi
 259:	8b 7c 24 1c          	mov    edi,DWORD PTR [esp+0x1c]
 25d:	8d 2c cd 00 00 00 00 	lea    ebp,[ecx*8+0x0]
 264:	29 cd                	sub    ebp,ecx
 266:	88 58 08             	mov    BYTE PTR [eax+0x8],bl
 269:	8b 0d 3a 00 00 00    	mov    ecx,DWORD PTR ds:0x3a
 26f:	c1 ee 04             	shr    esi,0x4
 272:	c1 e7 03             	shl    edi,0x3
 275:	83 c0 03             	add    eax,0x3
 278:	2b 7c 24 1c          	sub    edi,DWORD PTR [esp+0x1c]
 27c:	83 44 24 28 01       	add    DWORD PTR [esp+0x28],0x1
 281:	c1 ed 04             	shr    ebp,0x4
 284:	83 e9 01             	sub    ecx,0x1
 287:	c1 ef 04             	shr    edi,0x4
 28a:	83 44 24 34 03       	add    DWORD PTR [esp+0x34],0x3
 28f:	83 44 24 24 01       	add    DWORD PTR [esp+0x24],0x1
 294:	83 44 24 20 03       	add    DWORD PTR [esp+0x20],0x3
 299:	3b 4c 24 28          	cmp    ecx,DWORD PTR [esp+0x28]
 29d:	0f 87 35 fe ff ff    	ja     d8 <Dither+0xc8>
 2a3:	8b 4c 24 34          	mov    ecx,DWORD PTR [esp+0x34]
 2a7:	8b 5c 24 60          	mov    ebx,DWORD PTR [esp+0x60]
 2ab:	0f b6 04 0b          	movzx  eax,BYTE PTR [ebx+ecx*1]
 2af:	01 c6                	add    esi,eax
 2b1:	0f b6 44 0b 01       	movzx  eax,BYTE PTR [ebx+ecx*1+0x1]
 2b6:	01 c7                	add    edi,eax
 2b8:	0f b6 44 0b 02       	movzx  eax,BYTE PTR [ebx+ecx*1+0x2]
 2bd:	01 c5                	add    ebp,eax
 2bf:	66 81 fe ff 00       	cmp    si,0xff
 2c4:	0f 47 f2             	cmova  esi,edx
 2c7:	89 d0                	mov    eax,edx
 2c9:	c1 ee 06             	shr    esi,0x6
 2cc:	66 81 fd ff 00       	cmp    bp,0xff
 2d1:	0f 46 c5             	cmovbe eax,ebp
 2d4:	83 e0 e0             	and    eax,0xffffffe0
 2d7:	09 f0                	or     eax,esi
 2d9:	66 81 ff ff 00       	cmp    di,0xff
 2de:	8b 74 24 24          	mov    esi,DWORD PTR [esp+0x24]
 2e2:	0f 47 fa             	cmova  edi,edx
 2e5:	81 e7 e0 ff 00 00    	and    edi,0xffe0
 2eb:	c1 ff 03             	sar    edi,0x3
 2ee:	09 f8                	or     eax,edi
 2f0:	8b 7c 24 64          	mov    edi,DWORD PTR [esp+0x64]
 2f4:	83 44 24 38 01       	add    DWORD PTR [esp+0x38],0x1
 2f9:	88 04 37             	mov    BYTE PTR [edi+esi*1],al
 2fc:	89 c8                	mov    eax,ecx
 2fe:	8b 0d 3e 00 00 00    	mov    ecx,DWORD PTR ds:0x3e
 304:	83 c0 03             	add    eax,0x3
 307:	83 c6 01             	add    esi,0x1
 30a:	83 e9 01             	sub    ecx,0x1
 30d:	3b 4c 24 38          	cmp    ecx,DWORD PTR [esp+0x38]
 311:	0f 87 59 fd ff ff    	ja     70 <Dither+0x60>
 317:	8b 15 3a 00 00 00    	mov    edx,DWORD PTR ds:0x3a
 31d:	85 d2                	test   edx,edx
 31f:	0f 84 14 fd ff ff    	je     39 <Dither+0x29>
 325:	8b 6c 24 64          	mov    ebp,DWORD PTR [esp+0x64]
 329:	31 db                	xor    ebx,ebx
 32b:	03 44 24 60          	add    eax,DWORD PTR [esp+0x60]
 32f:	8d 74 35 00          	lea    esi,[ebp+esi*1+0x0]
 333:	90                   	nop
 334:	8d 74 26 00          	lea    esi,[esi+eiz*1+0x0]
 338:	0f b6 08             	movzx  ecx,BYTE PTR [eax]
 33b:	0f b6 50 02          	movzx  edx,BYTE PTR [eax+0x2]
 33f:	c1 f9 06             	sar    ecx,0x6
 342:	83 e2 e0             	and    edx,0xffffffe0
 345:	09 ca                	or     edx,ecx
 347:	0f b6 48 01          	movzx  ecx,BYTE PTR [eax+0x1]
 34b:	83 c0 03             	add    eax,0x3
 34e:	83 e1 e0             	and    ecx,0xffffffe0
 351:	c1 f9 03             	sar    ecx,0x3
 354:	09 ca                	or     edx,ecx
 356:	88 14 1e             	mov    BYTE PTR [esi+ebx*1],dl
 359:	8b 15 3a 00 00 00    	mov    edx,DWORD PTR ds:0x3a
 35f:	83 c3 01             	add    ebx,0x1
 362:	39 da                	cmp    edx,ebx
 364:	77 d2                	ja     338 <Dither+0x328>
 366:	83 c4 4c             	add    esp,0x4c
 369:	5b                   	pop    ebx
 36a:	5e                   	pop    esi
 36b:	5f                   	pop    edi
 36c:	5d                   	pop    ebp
 36d:	c3                   	ret    
 36e:	31 f6                	xor    esi,esi
 370:	31 ff                	xor    edi,edi
 372:	31 ed                	xor    ebp,ebp
 374:	e9 2a ff ff ff       	jmp    2a3 <Dither+0x293>
 379:	8b 0d 3a 00 00 00    	mov    ecx,DWORD PTR ds:0x3a
 37f:	a1 3e 00 00 00       	mov    eax,ds:0x3e
 384:	8b 15 42 00 00 00    	mov    edx,DWORD PTR ds:0x42
 38a:	0f af c1             	imul   eax,ecx
 38d:	0f af c2             	imul   eax,edx
 390:	8b 54 24 64          	mov    edx,DWORD PTR [esp+0x64]
 394:	c1 e8 03             	shr    eax,0x3
 397:	89 44 24 08          	mov    DWORD PTR [esp+0x8],eax
 39b:	8b 44 24 60          	mov    eax,DWORD PTR [esp+0x60]
 39f:	89 14 24             	mov    DWORD PTR [esp],edx
 3a2:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
 3a6:	e8 fc ff ff ff       	call   3a7 <Dither+0x397>
 3ab:	e9 89 fc ff ff       	jmp    39 <Dither+0x29>
