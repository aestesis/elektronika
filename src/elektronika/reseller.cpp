// regfile // elektronika

#include "password.h"
#include "main.h"
#include "computerInfo.h"
#include "interface.h"
#include "md5.h"

#ifdef RESELLER

static Tregfile regresell[]={
{ "92171dcd7643a470", 0x820820c2},
{ "5cb9d12f6be56100", 0x0},
{ "228e730bf63c2a00", 0x0},
{ "8d3e6bda16f5b800", 0x0},
{ "0167485b48e1e240", 0x0},
{ "fe1db44198139400", 0x0},
{ "20ab7fd800bbdffb", 0x0},
{ "9cc63be4017286c0", 0x0},
{ "5343c8cecc1874d8", 0x0},
{ "4af5fb6063e43000", 0x0},
{ "8ebc237f4deb71e8", 0x0},
{ "ca72740cd9ac5000", 0x0},
{ "0ace0e6702ae38c0", 0x0},
{ "ce836aafada501d0", 0x0},
{ "11cf5ca252f73950", 0x0},
{ "5b575b3f06e42548", 0x0},
{ "e251514528dee680", 0x0},
{ "b2ff84b587332ab0", 0x0},
{ "a71ac789d0d8c620", 0x0},
{ "06ac172cd6f9a662", 0x0},
{ "67bd4805062cc120", 0x0},
{ "8f135a0b7d7ed2e0", 0x0},
{ "b4d7d4667bf18b1e", 0x0},
{ "666bd9662039c258", 0x0},
{ "cdce13fa7aeb3300", 0x0},
{ "377da2136ed1e1e0", 0x0},
{ "15c873f5a7fbc8b0", 0x0},
{ "bc307515571b3c30", 0x0},
{ "b804efb063fe202a", 0x0},
{ "c31f7dd6a2468f00", 0x0},
{ "37d04abad434b5bc", 0x0},
{ "1728fe8b30125ec0", 0x0},
{ "83564b1b6f1c1cc0", 0x0},
{ "50cfb30429322ff0", 0x0},
{ "100d045edaa6a628", 0x0},
{ "755cb24a2e8ca804", 0x0},
{ "da885ec6a2c2628c", 0x0},
{ "145a25baae7524a0", 0x0},
{ "4878dbb7fa8dbe88", 0x0},
{ "41d64c1edf18ed90", 0x0},
{ "4857fc18d9d35d40", 0x0},
{ "81f9c83f5a353ab0", 0x0},
{ "f9f7473b8da53358", 0x0},
{ "d4e2a1c33eb97d6c", 0x0},
{ "4a7b2ac2ae65aec0", 0x0},
{ "b9718deae2f2bc7a", 0x0},
{ "608c64aeeb4d4480", 0x0},
{ "91845b15938cd438", 0x0},
{ "c481259acb6a0a80", 0x0},
{ "b89102d6dfad4360", 0x0},
{ "9cbbaa954ba08e80", 0x0},
{ "2d47bf314cb71764", 0x0},
{ "8c114323bf6b4480", 0x0},
{ "55a5d203e42f54c0", 0x0},
{ "3f6b8d6802ee6960", 0x0},
{ "04ec694081495d58", 0x0},
{ "2457017def636250", 0x0},
{ "7d15233312824370", 0x0},
{ "310c117d98465808", 0x0},
{ "b9c1774ec1801f90", 0x0},
{ "55902d152a62d200", 0x0},
{ "3d8f920e82e56750", 0x0},
{ "3cc0e035c1ef4406", 0x0},
{ "cd0136efbdd4b498", 0x0},
{ "d678d752163e5b40", 0x0},
{ "844ff4ab65c90fc0", 0x0},
{ "7f9ecb17d1405fb8", 0x0},
{ "68d735efeaaec900", 0x0},
{ "383f76b1ac61e9c0", 0x0},
{ "255ab152e62c6528", 0x0},
{ "b5e47f4ff98a18f0", 0x0},
{ "94de827a9acbbf80", 0x0},
{ "75027cbe7c8af0f4", 0x0},
{ "195a1cffb1699100", 0x0},
{ "e4ee56c18be9a328", 0x0},
{ "59c83512b4e96890", 0x0},
{ "d43ff3047cd38200", 0x0},
{ "52e2111d603c3400", 0x0},
{ "103a0f4972bc9400", 0x0},
{ "57202a7f38f04e30", 0x0},
{ "c2d1d9ce55067228", 0x0},
{ "8a1a10eca60ffd08", 0x0},
{ "0556a2c6e502d150", 0x0},
{ "4ed8b1ddd771eb40", 0x0},
{ "b5ff47578d38fffb", 0x0},
{ "22e8519151763b00", 0x0},
{ "72b13fd491d1df80", 0x0},
{ "e4b14a29c1ac5d86", 0x0},
{ "d4b9a1d581560294", 0x0},
{ "c5d09aca5c46fb60", 0x0},
{ "db4cf1a58442d6ce", 0x0},
{ "a288d99674c73a38", 0x0},
{ "50ae29d90423c868", 0x0},
{ "59818533963d167e", 0x0},
{ "2cb43594f7bfcdc0", 0x0},
{ "212f6351de2e2f16", 0x0},
{ "d4a2108b247148b0", 0x0},
{ "8ca99c3c744f5fc0", 0x0},
{ "ac42536ae6ad84e0", 0x0},
{ "90859aa7b490ae00", 0x0},
{ "590f250141a514aa", 0x0},
{ "8ba0c5247228a43c", 0x0},
{ "9ed9d213ae0d49ec", 0x0},
{ "dee5f6f943119224", 0x0},
{ "49c7afe91f7f7000", 0x0},
{ "0db8264ac6d85480", 0x0},
{ "2fb70b78adbd2be0", 0x0},
{ "3f665d820f269ec8", 0x0},
{ "fa1c9f1a7bb0f5b2", 0x0},
{ "b8bdfb052201d5dc", 0x0},
{ "e971589c66f2ec78", 0x0},
{ "fc0e30f77f6c27c6", 0x0},
{ "cdc2698d95562320", 0x0},
{ "66f5ddf53838aa00", 0x0},
{ "0ad3cdf459c77b60", 0x0},
{ "a8c87f2cfbaf4e00", 0x0},
{ "7ac6a3ea402ad6be", 0x0},
{ "03a4b1fb3c8de180", 0x0},
{ "84d4eab1efe82902", 0x0},
{ "c9006047b906e2e0", 0x0},
{ "2b1e5613c94f6d80", 0x0},
{ "6311b52a4aa2ac00", 0x0},
{ "fd7452a27e69f380", 0x0},
{ "4e39f32018634000", 0x0},
{ "585bcb2ca0979300", 0x0},
{ "a8665118f9574400", 0x0},
{ "2a13f147e7270ab8", 0x0},
{ "c61c72ff260ddb5e", 0x0},
{ "ceb7aa7710145be1", 0x0},
{ "9ff0b9f1497b11c0", 0x0},
{ "7a965ab86da33a80", 0x0},
{ "23550c30246a75d4", 0x0},
{ "db55f56e88d17f80", 0x0},
{ "461091fc97c0595e", 0x0},
{ "4c82c25e56ca0a58", 0x0},
{ "cf8ae42368b659e8", 0x0},
{ "b5d54f954327179c", 0x0},
{ "ec629311a4c22970", 0x0},
{ "5b200bdeaac7b270", 0x0},
{ "b3afcafe2045251c", 0x0},
{ "f643dd7db2f7d880", 0x0},
{ "5308a9eea20e005c", 0x0},
{ "0fe35611fab50e80", 0x0},
{ "91435374f94e0520", 0x0},
{ "00e7385c019fad08", 0x0},
{ "50fe1c5b351e4780", 0x0},
{ "767c112bb38b8600", 0x0},
{ "200d50b37ee9b1f0", 0x0},
{ "81670a943817bd5c", 0x0},
{ "8cd17ea922fe9180", 0x0},
{ "c4388b1aac60d200", 0x0},
{ "24a950d4b77e3280", 0x0},
{ "78eb97cef5c964e2", 0x0},
{ "3e2737523d802100", 0x0},
{ "d8d8e0411ec0aa00", 0x0},
{ "eac78fa8dbaf5b80", 0x0},
{ "7eeafec31831ca10", 0x0},
{ "f214e3540b3616d0", 0x0},
{ "1cd90b7b12351680", 0x0},
{ "eeca730c204ba520", 0x0},
{ "b130574431260568", 0x0},
{ "1d48e9f0aca071e0", 0x0},
{ "674c0a5425deb7c0", 0x0},
{ "1e136475e4c558a0", 0x0},
{ "6c9d6f159b9a05a0", 0x0},
{ "f391599d6e5e6f00", 0x0},
{ "7a8f36ddd135e58e", 0x0},
{ "0cb5ff84ffa494cc", 0x0},
{ "05b5adfc2b507400", 0x0},
{ "1ff99df98c3490c0", 0x0},
{ "432211365bf20b40", 0x0},
{ "8f7d57f88c7c5300", 0x0},
{ "1ad2d832f1b565f8", 0x0},
{ "8b60ea28c00e6000", 0x0},
{ "093beaca4dfc6c00", 0x0},
{ "5c6e5d3032bb8900", 0x0},
{ "93b81c8e3a9eaffc", 0x0},
{ "38ace6b5d5a1fd92", 0x0},
{ "e0b96b5d1a42e624", 0x0},
{ "8c247b9534dd0df0", 0x0},
{ "154aa45819a9e000", 0x0},
{ "cfa24e0ffc56dc00", 0x0},
{ "f7c2354e4fa82528", 0x0},
{ "670dc1d340fc8430", 0x0},
{ "482dd0d77f475380", 0x0},
{ "deff5fe487191ae0", 0x0},
{ "723017c1a905b800", 0x0},
{ "195bf68d836318a0", 0x0},
{ "2e35f7eb21a856a0", 0x0},
{ "c3f240aab2bcb586", 0x0},
{ "2a3870266ef86c00", 0x0},
{ "c4bc149fbeb61bc4", 0x0},
{ "cf0717be98aea920", 0x0},
{ "ece414dca2c8cf00", 0x0},
{ "f3e4994a0ad25e14", 0x0},
{ "19f3d37518a65bc8", 0x0},
{ "54393f490669cea8", 0x0},
{ "767a7f16803fa350", 0x0},
{ "c1ac044233863944", 0x0},
{ "09a6b709b8e44e4a", 0x0},
{ "9284cd2f3812fa00", 0x0},
{ "156fdafc1cdf2180", 0x0},
{ "c066bb50e5a70110", 0x0},
{ "0c7488edd278c800", 0x0},
{ "b2b60ffcbf3cf000", 0x0},
{ "7f68ac7d8134bf88", 0x0},
{ "901d6c291bf2c960", 0x0},
{ "209927f936646c90", 0x0},
{ "7e88f1af74fb2cc0", 0x0},
{ "cb6ece399fb37540", 0x0},
{ "64d7d27dc0140ef8", 0x0},
{ "da5a1f4b5a4e4480", 0x0},
{ "a54b8d38cfb0ade0", 0x0},
{ "f7abd59be9a1b476", 0x0},
{ "b945e46f64162400", 0x0},
{ "c1003932f06ff200", 0x0},
{ "9064340b1e5d3b50", 0x0},
{ "3dedcd9c736911c0", 0x0},
{ "bbb03a8f6a7a4bc0", 0x0},
{ "6c23eece1b919960", 0x0},
{ "45e538ef6ad5d320", 0x0},
{ "15ce1bb91779b5f4", 0x0},
{ "b7d41888d2462372", 0x0},
{ "49db431e239fafa0", 0x0},
{ "82be9ca4c51bdac0", 0x0},
{ "b700e43ad7962e00", 0x0},
{ "d2f7499bd280da00", 0x0},
{ "6359ab361d24a3f0", 0x0},
{ "7de8fcf7092ca7a0", 0x0},
{ "e1231cf1226f0994", 0x0},
{ "b149a3a5195cb6fc", 0x0},
{ "015ac63f59fce680", 0x0},
{ "c09beddb0e6ef0dc", 0x0},
{ "61b88e4d7ef8f440", 0x0},
{ "c3613fec7a182668", 0x0},
{ "ceef0f801d513000", 0x0},
{ "4294ed08efb870da", 0x0},
{ "53426ae6cbcbe8e6", 0x0},
{ "56cf0b2ade009460", 0x0},
{ "7fcc56db0f553bac", 0x0},
{ "0375de8c2e995844", 0x0},
{ "13888b9a47fa1700", 0x0},
{ "e7bef2121243ec68", 0x0},
{ "f363d87bd5201b00", 0x0},
{ "c5140c7a0586abce", 0x0},
{ "6b3a5ae012cd0100", 0x0},
{ "e4c9f09d939bb3c0", 0x0},
{ "37064527f59e9228", 0x0},
{ "3401dcce10a711a0", 0x0},
{ "9cf31e8d40c9cd1e", 0x0},
{ "c35dc235a20c4198", 0x0},
{ "ad03b944637969c0", 0x0},
{ "ea25de060bcfd120", 0x0},
{ "e170387bb54997d4", 0x0},
{ "e63112cfb412ce40", 0x0},
{ "26cdd30a88d71400", 0x0},
{ "3cda8de79d8d2028", 0x0},
{ "2c7b82e2c9eb2780", 0x0},
{ "01236cd9b031e2f4", 0x0},
{ "b575f277e2d64de0", 0x0},
{ "5e96c4f3c0922b00", 0x0},
{ "e0a7f9a58578b510", 0x0},
{ "f4d92f84d0913d80", 0x0},
{ "3087af41ff697b00", 0x0},
{ "fc2b2de7f0634540", 0x0},
{ "ca6815b019f0b6be", 0x0},
{ "264aae66afd20e88", 0x0},
{ "10299fb2305f477c", 0x0},
{ "a1b55bf5abeefad3", 0x0},
{ "6c3f8f303e84f334", 0x0},
{ "cd5114947c90eb58", 0x0},
{ "15059bb9de552f00", 0x0},
{ "d43dfb13b68742c0", 0x0},
{ "4bde74f8126be378", 0x0},
{ "acef38a936aa0b84", 0x0},
{ "57dbb653d79fbb80", 0x0},
{ "61c60aeb34e845d0", 0x0},
{ "52fe7ee9caaced74", 0x0},
{ "f1e5c59db2605180", 0x0},
{ "a07e43beb9684280", 0x0},
{ "5ded23d07c042610", 0x0},
{ "237453b0ed24aee0", 0x0},
{ "e95488791f257bf0", 0x0},
{ "e98a8eddd08e2354", 0x0},
{ "6174c140cadb34d0", 0x0},
{ "9f86b54ffecb8290", 0x0},
{ "fe1d3976968e77a4", 0x0},
{ "58696b74bbe29054", 0x0},
{ "f99a276f77f175d0", 0x0},
{ "9e8b447c78534ab4", 0x0},
{ "57c3fb0c047c1858", 0x0},
{ "5385254b7a2127d0", 0x0},
{ "0e63252b5106b01b", 0x0},
{ "73b30e2c36617bb9", 0x0},
{ "7fb98580e58e7f24", 0x0},
{ "5772ca4b3b7ada80", 0x0},
{ "56e139ba180947e0", 0x0},
{ "3a68ea0245d90740", 0x0},
{ "f881e1fe93299254", 0x0},
{ "0f2f0ed85faf7600", 0x0},
{ "f49b167bc02c4290", 0x0},
{ "defb80b4d4f15360", 0x0},
{ "3a2f51279f959a80", 0x0},
{ "7b6ccc393621a7c0", 0x0},
{ "4e3be6d40eaf0f26", 0x0},
{ "ae1615789ed14720", 0x0},
{ "ae28e728daa44ee0", 0x0},
{ "28653c337d532500", 0x0},
{ "b665468e43eb7278", 0x0},
{ "4de4f9dd3ba5dc58", 0x0},
{ "a8dab0db362d3000", 0x0},
{ "a6152c542b7e47d7", 0x0},
{ "9b7f3b3e90cfcac0", 0x0},
{ "2b5364408f2a3b20", 0x0},
{ "d10583b4ebd9a3f3", 0x0},
{ "b279444dafcaf5da", 0x0},
{ "12580a325a609400", 0x0},
{ "b9ba55b2b0c4f274", 0x0},
{ "9c3e153874b44327", 0x0},
{ "044828ec39c95a00", 0x0},
{ "87b78ebcb72a6ca8", 0x0},
{ "fc6283f10e98fcb0", 0x0},
{ "77ec0521523141ce", 0x0},
{ "b7b80d25878d74a0", 0x0},
{ "b10b437cdcfb1070", 0x0},
{ "8566d8254a798800", 0x0},
{ "f5bc53408fd49b3a", 0x0},
{ "62544bb7ad85cf40", 0x0},
{ "397c7a89529b99c0", 0x0},
{ "30b9d0928d014868", 0x0},
{ "774ddd520f1ea89d", 0x0},
{ "73ff57f84b61f000", 0x0},
{ "8cc184f858c69980", 0x0},
{ "15a0b4ecd59e7740", 0x0},
{ "33739ec79a59f100", 0x0},
{ "76aeee028dbb6038", 0x0},
{ "55397658f8b1f900", 0x0},
{ "f3054429148a528c", 0x0},
{ "d9143b6b757968e8", 0x0},
{ "2d2e3857593af000", 0x0},
{ "1d25db12df26d248", 0x0},
{ "6433a2e713e1e4e0", 0x0},
{ "d62092aa29359700", 0x0},
{ "b6bfda022e3f4b84", 0x0},
{ "7304986124c43880", 0x0},
{ "081853593cde86a0", 0x0},
{ "e0dca888cbb4da00", 0x0},
{ "11d1cb9dccd44f70", 0x0},
{ "0c0afe5cbaeb3140", 0x0},
{ "ed4e3fd7797161b4", 0x0},
{ "632da7aa04474648", 0x0},
{ "4cd895a54abf9000", 0x0},
{ "5b3609fe49a62694", 0x0},
{ "aced988b1e97f3ba", 0x0},
{ "392424cb0efd289d", 0x0},
{ "283c42af6f402b80", 0x0},
{ "12fbd3bbaffd0800", 0x0},
{ "018a7a955d7505e0", 0x0},
{ "94c90e8427d32c00", 0x0},
{ "0d136f2d1fecc940", 0x0},
{ "6d813842b8efc288", 0x0},
{ "ff415fd760c67b00", 0x0},
{ "2a1fd7b1d0a7a220", 0x0},
{ "fc3b98af85fcedd8", 0x0},
{ "dffeabb0226d6d00", 0x0},
{ "858aa821e8cdc260", 0x0},
{ "3a0b3c76a9ae9e10", 0x0},
{ "7d7823aaac8c8ba0", 0x0},
{ "cefa725c689b7c30", 0x0},
{ "52a82f126e0931c0", 0x0},
{ "0c059f20cb1f3500", 0x0},
{ "24de9f21d54f30e0", 0x0},
{ "4f16a62203e5f986", 0x0},
{ "db08d67ac253e540", 0x0},
{ "54ef180cb75d5ddc", 0x0},
{ "5a07856d445a6998", 0x0},
{ "110bbf87ecbc5440", 0x0},
{ "d9db332cc91a9540", 0x0},
{ "f56483c48bdc4000", 0x0},
{ "da8ad7ed929f060c", 0x0},
{ "2d1ee287f88c3ef0", 0x0},
{ "b94eff4db121a304", 0x0},
{ "4cec795d6e5c8000", 0x0},
{ "d3b447fabe32e440", 0x0},
{ "67eee1cd3a9b7ca0", 0x0},
{ "2b9feba78b114f80", 0x0},
{ "4650a6d2c98bb40e", 0x0},
{ "330ffdc929fa19b0", 0x0},
{ "1928f5b8bbd86878", 0x0},
{ "d165df902f258f40", 0x0},
{ "41b45e2e303100dc", 0x0},
{ "8c288cdb8b1c0800", 0x0},
{ "e1f9ef0a12278b80", 0x0},
{ "6ec27c03d77306a8", 0x0},
{ "68ae632e26396ee0", 0x0},
{ "618e0a1cd906f7c0", 0x0},
{ "9cd8a6d909d2b070", 0x0},
{ "0508df5d7538d800", 0x0},
{ "2c9d03d1e3034000", 0x0},
{ "4a8fabb921cbbe48", 0x0},
{ "4d2c2ac90af3dc00", 0x0},
{ "d702a5ef63a51578", 0x0},
{ "e179d678872dfc00", 0x0},
{ "6bb3f0ff70fe2bc0", 0x0},
{ "4172c33c39d50620", 0x0},
{ "081ef5f118d55380", 0x0},
{ "f51eb38759726956", 0x0},
{ "127e66d2b7a1a1e0", 0x0},
{ "145c64a81ab410d0", 0x0},
{ "14b6f21cf76ac2c0", 0x0},
{ "278185e33cf02fb8", 0x0},
{ "06a02d74ec005900", 0x0},
{ "097cdab6af0d688c", 0x0},
{ "3347e999d0493eb8", 0x0},
{ "53bb314f45172c04", 0x0},
{ "9e13e75c6d349be0", 0x0},
{ "416ed10add22b154", 0x0},
{ "b7bfede127064500", 0x0},
{ "99435ebea5b47480", 0x0},
{ "bc273d80bc9043c0", 0x0},
{ "d1db33f81361a9c0", 0x0},
{ "af659fb407ab7160", 0x0},
{ "2e71117a1281bb90", 0x0},
{ "b457d1f9863a9ca0", 0x0},
{ "310227aca50d7000", 0x0},
{ "6e80234e88b7a000", 0x0},
{ "53a61fb69f9010e0", 0x0},
{ "d95963823186c49e", 0x0},
{ "2ee0cd0bc2d94f80", 0x0},
{ "0a77377754b94770", 0x0},
{ "e8f350554793be00", 0x0},
{ "4f8a857b71c6e580", 0x0},
{ "4f12dc2742283190", 0x0},
{ "7e097b4f81af0da0", 0x0},
{ "66871197dfbaa7a2", 0x0},
{ "8cac1520d630b010", 0x0},
{ "616ab7f1405f16a0", 0x0},
{ "600fc8f11dd9c600", 0x0},
{ "89ca49064f887700", 0x0},
{ "7860660b7c72b800", 0x0},
{ "2c454cf1aa9e26d8", 0x0},
{ "dacdd2b3ebadc2bc", 0x0},
{ "4dbdfd52d656d5e0", 0x0},
{ "28c606a0930502e0", 0x0},
{ "529f61cee5d2b60c", 0x0},
{ "b6fce5e37620a5a0", 0x0},
{ "59cf0a33dcbeb5c0", 0x0},
{ "230e4a1bedba3920", 0x0},
{ "6c2251b016fe7200", 0x0},
{ "ed80b7a420501418", 0x0},
{ "2d03f0a11e6560b8", 0x0},
{ "28a9db38f2293800", 0x0},
{ "907e3f4054e41250", 0x0},
{ "1707ed274971b3e0", 0x0},
{ "fb96a66d43a8a8e0", 0x0},
{ "ceea1d375529c9a0", 0x0},
{ "c53cc51ff9640c30", 0x0},
{ "e7af628d19eb0948", 0x0},
{ "b4f994d0efdd5320", 0x0},
{ "22a0475bca54b010", 0x0},
{ "055b0fee19bdba10", 0x0},
{ "bb78dee9d3ffcaa4", 0x0},
{ "b72a989c08cb22a0", 0x0},
{ "ca8125c098818a00", 0x0},
{ "4359ae7265b00a78", 0x0},
{ "51e2a8a9261c2600", 0x0},
{ "4bda8c094b7a4d00", 0x0},
{ "1788f27584526180", 0x0},
{ "8e8ebe0176d02078", 0x0},
{ "5fd9dfc5365a1034", 0x0},
{ "deb5590ac9a95094", 0x0},
{ "2d415a02b629ea00", 0x0},
{ "8ff1b853f83eadf0", 0x0},
{ "9e791b0fe8b9b7b0", 0x0},
{ "252894de04af2d6a", 0x0},
{ "438e26692f77c280", 0x0},
{ "af32f994e7fdf650", 0x0},
{ "aadb65c336378138", 0x0},
{ "1387c484c2623d00", 0x0},
{ "7f3984238230fce8", 0x0},
{ "b4b449c776aec280", 0x0},
{ "0f10d45b9de7244c", 0x0},
{ "4940021e7995d900", 0x0},
{ "1eaeef052c4d5b40", 0x0},
{ "5540f91d766238d0", 0x0},
{ "0e413582c981c7c4", 0x0},
{ "e0e31cd7b0a46b4a", 0x0},
{ "276bea35b8bf9580", 0x0},
{ "b6a57a595f98a280", 0x0},
{ "5f04e3e6484d4e00", 0x0},
{ "1295642ad72c36e0", 0x0},
{ "7fe032b59bd4dae0", 0x0},
{ "f1caf38aa83c74d8", 0x0},
{ "1151a6fe68027b20", 0x0},
{ "25594c1d19729dee", 0x0},
{ "22ff3445b32f5ec8", 0x0},
{ "c81a0b72294045dc", 0x0},
{ "9cea4bc54ce0cf80", 0x0},
{ "c12b913f0f88b1bc", 0x0},
{ "e40c80ff1a1bd9d0", 0x0},
{ "3aecc0f50f56fa06", 0x0},
{ "ef84e87e6fd6a928", 0x0},
{ "ec29f937a01c8304", 0x0},
{ "0993d75a0fd9cf00", 0x0},
{ "87aa55e32d392700", 0x0},
{ "c01a87ccb8deaf80", 0x0},
{ "50e4ba8492ee4560", 0x0},
{ "3b6db94cfd21f3f8", 0x0},
{ "0d645ae27ee6ab40", 0x0},
{ "221219f8b1a0e7fc", 0x0},
{ "a45c408517f29c44", 0x0},
{ "3d0d9bc2b2100b00", 0x0},
{ "7bd16d9dac7424f0", 0x0},
{ "675e9a355746d800", 0x0},
{ "8b9bc55c92b8b780", 0x0},
{ "25728100b86fbce8", 0x0},
{ "add390331b612a60", 0x0},
{ "133d3169dec79df4", 0x0},
{ "90378e83310866c0", 0x0},
{ "dd80a9100eb46c8b", 0x0},
{ "b707ecbb1f1c3000", 0x0},
{ "3cb2b8bd8d71b130", 0x0},
{ "f94f35c96daa1050", 0x0},
{ "0647ad622822720a", 0x0},
{ "a2f4592dc70c0de0", 0x0},
{ "33cf61bdc1d6d380", 0x0},
{ "9b9ae019ee927ff0", 0x0},
{ "fe9a25af2fe77f00", 0x0},
{ "c00166d1d92b9a28", 0x0},
{ "d6c06dbab4dae9b0", 0x0},
{ "0f5365f3a364d220", 0x0},
{ "6821af65187919b0", 0x0},
{ "23dc876a10b4f4b1", 0x0},
{ "f57fa5543086c6e6", 0x0},
{ "6de3276bf3ae8d80", 0x0},
{ "6eeca6d95102dd00", 0x0},
{ "cd474fe07c68e918", 0x0},
{ "ad7696ec9e1d2b40", 0x0},
{ "2f8d5dffa7a40780", 0x0},
{ "003b6f401789c900", 0x0},
{ "16e4c82d963956f4", 0x0},
{ "275314d54dc07800", 0x0},
{ "64227556bb96bfc0", 0x0},
{ "f3faecc7b23394e3", 0x0},
{ "c62111f96a6876f0", 0x0},
{ "660b17534a850400", 0x0},
{ "8ab30a440c705a80", 0x0},
{ "fa97239139222660", 0x0},
{ "13a425fd583a5800", 0x0},
{ "38eed6e9d2e98e00", 0x0},
{ "81cab122147ac146", 0x0},
{ "0fdc23bf5df6a354", 0x0},
{ "92dc1896e5aeae40", 0x0},
{ "23c6f90a41d0cd80", 0x0},
{ "35f6c2f49793f000", 0x0},
{ "b43dd71a5b03a100", 0x0},
{ "0292628df4a083e0", 0x0},
{ "7be5e4ab26b84be0", 0x0},
{ "f3d25338ddded740", 0x0},
{ "7aa0dc88c9d14048", 0x0},
{ "3e1ebbd0328e6bcc", 0x0},
{ "7821745c8d9b4f78", 0x0},
{ "db48d1e04d991ef0", 0x0},
{ "1edced1e36297328", 0x0},
{ "2317791c617ee140", 0x0},
{ "e93d35ca43d9a900", 0x0},
{ "c934f598cf2a1d34", 0x0},
{ "c3d0a32faaaf7780", 0x0},
{ "c7952edd7df6a7e0", 0x0},
{ "872747032be8da88", 0x0},
{ "26c93e700bd89730", 0x0},
{ "1ce7ad97350d9b00", 0x0},
{ "7cfe654bc9729109", 0x0},
{ "6b0c8b11a0609232", 0x0},
{ "1edfb4a4ee515240", 0x0},
{ "c78f0e121d1602f0", 0x0},
{ "e508c5afaabdc1c0", 0x0},
{ "a892b4cc8c3a3540", 0x0},
{ "104a3fc0bfc6e910", 0x0},
{ "80a7a7ab665b40e0", 0x0},
{ "2968d88aa4355a40", 0x0},
{ "6fc5e2383e4e6613", 0x0},
{ "345e9292879fc60b", 0x0},
{ "00ce4138b559c000", 0x0},
{ "4b414f461d08a200", 0x0},
{ "1d5d9a067f3e5ea8", 0x0},
{ "1dda1044e2951818", 0x0},
{ "ce8305526561ad00", 0x0},
{ "04c2027dc42aeb80", 0x0},
{ "ac8809aca575ce40", 0x0},
{ "1ce4a43548946560", 0x0},
{ "75c96032dd6b4434", 0x0},
{ "580c8c525c308800", 0x0},
{ "082e248173407d96", 0x0},
{ "94a38698c73f7400", 0x0},
{ "2bd1d8cd2f95dd90", 0x0},
{ "4e7642385c4a0fc0", 0x0},
{ "ab35febe2e777228", 0x0},
{ "fe9dd660953b7434", 0x0},
{ "988a706654e80620", 0x0},
{ "8e1bbb22e87b2000", 0x0},
{ "47b3c38993a62b70", 0x0},
{ "c911bf549ffb5a40", 0x0},
{ "9ebb04372c42feca", 0x0},
{ "5eaec4373b829440", 0x0},
{ "b70f4c62e2cdf480", 0x0},
{ "5e1da698d6a8a9c0", 0x0},
{ "913966e6c9ac5aa9", 0x0},
{ "a19c9b63bba5e710", 0x0},
{ "23ee95b3807a1ee0", 0x0},
{ "55a63d6525614534", 0x0},
{ "911a25b9826055c0", 0x0},
{ "727abf06dcc11a90", 0x0},
{ "a7684c7298662e80", 0x0},
{ "ebacae1c05806814", 0x0},
{ "d8b7a48731678550", 0x0},
{ "0ae33f38c56818cc", 0x0},
{ "4e8f573d0e41d402", 0x0},
{ "a41fd885d6f21cc0", 0x0},
{ "74ac0fc5f3332a64", 0x0},
{ "2035556b48aa9460", 0x0},
{ "eedf66989f07dca0", 0x0},
{ "1ad5c22959fdd910", 0x0},
{ "e03a06ad5e57d800", 0x0},
{ "007251a40ff4771c", 0x0},
{ "2bd97261a0d3ce00", 0x0},
{ "db934a04591f0a70", 0x0},
{ "c1b3981e4ee12aec", 0x0},
{ "a71ad056d7b35a00", 0x0},
{ "35acf6b96f90d9ee", 0x0},
{ "1bb941f3f4d60e70", 0x0},
{ "7c4f44c858c02e40", 0x0},
{ "ac5dd03fc6fcd7f6", 0x0},
{ "a1d0e891c10d41c0", 0x0},
{ "95bbd4191015f870", 0x0},
{ "5c75c5729c7f8b80", 0x0},
{ "ded9fdc37aa18000", 0x0},
{ "bd63722209241a10", 0x0},
{ "d3ef5fae487c75b0", 0x0},
{ "f0bda693348abc46", 0x0},
{ "b8feffdcbd0cec00", 0x0},
{ "c7b7c0782e647580", 0x0},
{ "43ea4cb553d324c0", 0x0},
{ "e3ea7d47e498ae3c", 0x0},
{ "8747eb1f7c004ec0", 0x0},
{ "a1f4a0dcb79a00ac", 0x0},
{ "d32d3ed0e54f6200", 0x0},
{ "84579868aa2cc1a0", 0x0},
{ "bc55f594c37bd240", 0x0},
{ "077aaf1c16a37400", 0x0},
{ "89f2dbc9791e70fa", 0x0},
{ "1c9f4c735649877c", 0x0},
{ "eaba0eb73c35ec10", 0x0},
{ "972e68b2bd6a36ec", 0x0},
{ "524f7bfff9724000", 0x0},
{ "0cbfc8260b5e0a60", 0x0},
{ "71153229ba667da8", 0x0},
{ "d0f6d6ff9288a800", 0x0},
{ "4ddd9d23a535a1f0", 0x0},
{ "35c68f3b0c566000", 0x0},
{ "290f6209891382a0", 0x0},
{ "5d6d22e25002c360", 0x0},
{ "34dbcab46dd64100", 0x0},
{ "bc228256fd3bc428", 0x0},
{ "89d5cabd5ef23fd4", 0x0},
{ "7ac956ccbae0e8e0", 0x0},
{ "dc6a2d514c0b7a00", 0x0},
{ "2a286c7944398120", 0x0},
{ "c5f1b18bd4fb9600", 0x0},
{ "2f369a5d45961ca8", 0x0},
{ "13c281fc98bf5fd4", 0x0},
{ "bd8172847fe39eb8", 0x0},
{ "89c1449a4d839c00", 0x0},
{ "992b5b65fe4225d0", 0x0},
{ "c5ee5eda1c551b10", 0x0},
{ "4c7c307546e4e1c0", 0x0},
{ "0a1b033faa495ddd", 0x0},
{ "603b2f8bf6ec8540", 0x0},
{ "778ff2210c1a8680", 0x0},
{ "b0fcf9fe741549d0", 0x0},
{ "5bf773f2b4e8f43e", 0x0},
{ "9d1396a357728cd8", 0x0},
{ "9d460260ec1809d3", 0x0},
{ "3219a58d0891f298", 0x0},
{ "a82e2e333335fdf8", 0x0},
{ "a86c7e9bec5c0680", 0x0},
{ "0dfaffe6135d10cf", 0x0},
{ "f0b84989a2d9f5b2", 0x0},
{ "9635d984628f508e", 0x0},
{ "2098b65ac83760c0", 0x0},
{ "18cabc693f99c000", 0x0},
{ "09fa85bca11804e4", 0x0},
{ "9d6b664f7fcc3800", 0x0},
{ "2f903f0d32e400fe", 0x0},
{ "7ec9c837cada7ed2", 0x0},
{ "6ada5ed58588c120", 0x0},
{ "468af2d9d903848c", 0x0},
{ "86d5a7ebdd1b8090", 0x0},
{ "11e11158f01dacd8", 0x0},
{ "8daa001b3df7c3c0", 0x0},
{ "b8d4d2bf1696bf3c", 0x0},
{ "d5eb59ae9a52c900", 0x0},
{ "aa635a1f0fb4984c", 0x0},
{ "97235faf0f022000", 0x0},
{ "eeabd5245c9a3318", 0x0},
{ "06eda6ecd9847220", 0x0},
{ "570784091212d0c0", 0x0},
{ "e0f0044fda420000", 0x0},
{ "f9bcab1b9db0c6e0", 0x0},
{ "9197d42961ae8ec8", 0x0},
{ "4f0d38fe1daad318", 0x0},
{ "4a318df86d4fa310", 0x0},
{ "5ef9c8e8a3c7bc40", 0x0},
{ "1125f2aeaf180ba0", 0x0},
{ "259e26a8f5f4bd68", 0x0},
{ "09d26b24a4aeef68", 0x0},
{ "201319552db391e0", 0x0},
{ "5541353ea2c36b00", 0x0},
{ "6eaa506641bb0f50", 0x0},
{ "b7bf6d0590e40e40", 0x0},
{ "924bfd7e70f02e50", 0x0},
{ "fe94be72a3f88b00", 0x0},
{ "73ba910863bb71b4", 0x0},
{ "35b12b97ca2b7ce0", 0x0},
{ "840226d15acf3640", 0x0},
{ "23f48bea84813b80", 0x0},
{ "e8f17ef737da09f8", 0x0},
{ "779ffa5ffd862e02", 0x0},
{ "2344e6addc00b1a2", 0x0},
{ "3e9dba4f94122400", 0x0},
{ "b035c451d36b3730", 0x0},
{ "73459430dc3d5d00", 0x0},
{ "3572497ce6cbc530", 0x0},
{ "d89e52777b1e61e0", 0x0},
{ "58b07ed5314b81a2", 0x0},
{ "2bc58550b9be22a0", 0x0},
{ "da4eba01a31e2960", 0x0},
{ "1b3ca5eed70b4ce0", 0x0},
{ "ad6b16f4b0df4480", 0x0},
{ "c989f2aa108817a0", 0x0},
{ "8fcae2e13cff3110", 0x0},
{ "e6bb0504a7de9688", 0x0},
{ "c8cc48e99c859c43", 0x0},
{ "0df45e3a3f31a0e0", 0x0},
{ "91dcb8777f79a1e9", 0x0},
{ "d130e7cf84f8a706", 0x0},
{ "6f64542665c26edc", 0x0},
{ "9f215cd2d5ecfdb0", 0x0},
{ "64bf6cfb580c42c0", 0x0},
{ "31dded1ef3298000", 0x0},
{ "25a49c358722eb90", 0x0},
{ "d9011e2add1e1860", 0x0},
{ "0c1b6e752e2127a3", 0x0},
{ "b812ab45bb9dcfd8", 0x0},
{ "15843a936b45d840", 0x0},
{ "e989017f45b7bc9a", 0x0},
{ "7d4348bc08d38d30", 0x0},
{ "d3dd16222068c660", 0x0},
{ "25ad3df6dc28ada6", 0x0},
{ "174163693c100120", 0x0},
{ "208260d79a464910", 0x0},
{ "c01f430d326f2a80", 0x0},
{ "addc1170d3974b00", 0x0},
{ "aa2da30fe21dfa00", 0x0},
{ "64a4a404b64a4a28", 0x0},
{ "fb4d43c3768fa620", 0x0},
{ "29a37a77a101b920", 0x0},
{ "83b95177041c2e00", 0x0},
{ "ee106bcea65063f0", 0x0},
{ "4d0086795d4d27f0", 0x0},
{ "76909b01d50c3544", 0x0},
{ "2b4ff621e8ee53a0", 0x0},
{ "9a5a362f49d544e0", 0x0},
{ "0e8c39a3db67e980", 0x0},
{ "cf249c703a96a3e0", 0x0},
{ "f06ffe5f12291b30", 0x0},
{ "b7cfe4ebbfbe6210", 0x0},
{ "323dc95ffd502000", 0x0},
{ "9640c6686f2f3980", 0x0},
{ "408098f9fe1385ec", 0x0},
{ "1faa703706db1900", 0x0},
{ "496d16ab8222a330", 0x0},
{ "5d4836f735f61008", 0x0},
{ "2af9db1b52e34960", 0x0},
{ "013206bc5cd7b700", 0x0},
{ "4d23b4193db6f380", 0x0},
{ "f85e2f154aeb0680", 0x0},
{ "b34fe0491d00c6c0", 0x0},
{ "ba12507d123ab2a0", 0x0},
{ "deb78ade81a0c968", 0x0},
{ "8793eca4f71db4c8", 0x0},
{ "7faaefe3d0aa7a30", 0x0},
{ "59e75616319c4080", 0x0},
{ "980977c00d442fc2", 0x0},
{ "17dc0efd3e58db00", 0x0},
{ "419f6254f615ed50", 0x0},
{ "4a8b3a225c066b00", 0x0},
{ "cce9605fa027c868", 0x0},
{ "7376e24e2e2118c0", 0x0},
{ "54654a2d359500c0", 0x0},
{ "5c32a348acdb5600", 0x0},
{ "9cd3b95c88494730", 0x0},
{ "232a83cf249f1b80", 0x0},
{ "f536657ae41138b0", 0x0},
{ "dd29fdff983d1d74", 0x0},
{ "e6bf6925e225d6e0", 0x0},
{ "e184af96c4d33cc0", 0x0},
{ "3a7ab93628c0eb40", 0x0},
{ "13e036ef2a044300", 0x0},
{ "196325e4c125f8d8", 0x0},
{ "3694666e762ad97c", 0x0},
{ "429ee1edc36080b8", 0x0},
{ "f4cf888ac39681eb", 0x0},
{ "0c96a277040c8186", 0x0},
{ "603c5357c64f2060", 0x0},
{ "7a61790228674220", 0x0},
{ "a2a8f51feb2bc000", 0x0},
{ "7f2e65140bc26040", 0x0},
{ "3b4935bba75a0000", 0x0},
{ "73e90705bf64fe36", 0x0},
{ "8ac60f6e93d33400", 0x0},
{ "cecf9b8e6a51cdac", 0x0},
{ "a4d1832fc1cb2410", 0x0},
{ "0691170c0c7ae500", 0x0},
{ "345d05bda520a8fc", 0x0},
{ "a15176039a26ad80", 0x0},
{ "a9fb45bf2a250780", 0x0},
{ "e19058e2fe3d568c", 0x0},
{ "4561268267a53bec", 0x0},
{ "03aa97ea51e7c400", 0x0},
{ "10d68eee44d14000", 0x0},
{ "248e2e1f4ee58b30", 0x0},
{ "2a5c627325bcdc18", 0x0},
{ "0fac8d3b698c8b40", 0x0},
{ "42e2656fce928900", 0x0},
{ "d5552a5bf9c0c8e0", 0x0},
{ "e0522a1fcdc32b78", 0x0},
{ "4ae5872b89b6f968", 0x0},
{ "ab4e7b2d8b9f5d00", 0x0},
{ "c30ff7d39ebb5278", 0x0},
{ "8a9f185f55690268", 0x0},
{ "5a023651a2b481b8", 0x0},
{ "141a9380e2461e10", 0x0},
{ "f545a8f5df65f320", 0x0},
{ "3d8f795e2bdfc048", 0x0},
{ "c296ffb2c80db700", 0x0},
{ "b4d4cafd89552b38", 0x0},
{ "bc61a36f96e68cc8", 0x0},
{ "328f7d4dae700eb0", 0x0},
{ "c37cd96761a56d28", 0x0},
{ "95118f89e76e6c3c", 0x0},
{ "43ce019cc6ec5600", 0x0},
{ "7e8f8eb14cd4ca00", 0x0},
{ "87d7313b7f8694f2", 0x0},
{ "daf5e9fd3e936882", 0x0},
{ "be5b20ccff741b50", 0x0},
{ "05a9e2e589588ebc", 0x0},
{ "5922ce0aca1efb76", 0x0},
{ "1868e9ad619f7c1c", 0x0},
{ "6b94b9265b49ed80", 0x0},
{ "825c64d5bd210540", 0x0},
{ "31d407b49dc717a0", 0x0},
{ "dfc35632b01e2d70", 0x0},
{ "fe2f96b31b57c660", 0x0},
{ "39a176e26c0c27b0", 0x0},
{ "020611fa1a10ab48", 0x0},
{ "04660138ceaa7500", 0x0},
{ "fc929547c52f4480", 0x0},
{ "1f79b47888593a86", 0x0},
{ "630f429f6e19a388", 0x0},
{ "3799a6744dc3dac0", 0x0},
{ "55729745c20d1280", 0x0},
{ "35fd21c74affe0b4", 0x0},
{ "b56d8abf1522d440", 0x0},
{ "806b57fb06590710", 0x0},
{ "ece86ca5d6bb7aa0", 0x0},
{ "50499d03fb9b42e0", 0x0},
{ "27c43d43f6f4d5c0", 0x0},
{ "4589f2316f64d838", 0x0},
{ "c96275a43096c358", 0x0},
{ "b4d98eb6b3b6c000", 0x0},
{ "ac84bcccef2fb86c", 0x0},
{ "1d00393107f139c0", 0x0},
{ "17d846ebad6e7948", 0x0},
{ "b6dd618689a2e7e0", 0x0},
{ "46325aaac67ed660", 0x0},
{ "9c27ff3ddc27a480", 0x0},
{ "b750e21e400282a0", 0x0},
{ "098b1c7909d69c00", 0x0},
{ "0593398960c8905a", 0x0},
{ "4429f44175c51120", 0x0},
{ "02b20d82f4019400", 0x0},
{ "44c1c388efb9ce00", 0x0},
{ "6ee5dfcadcb690f0", 0x0},
{ "76369aafc804cddc", 0x0},
{ "5b1e4260b1167d64", 0x0},
{ "6e6b0d9212fdbba4", 0x0},
{ "51b252878c7f2e00", 0x0},
{ "2bdcc2ee18eb0280", 0x0},
{ "034acc3573a59358", 0x0},
{ "b5abc1abf82c81d0", 0x0},
{ "b95aec8004c56968", 0x0},
{ "9e738ef630b2c6b4", 0x0},
{ "15fec2a3ce0b4480", 0x0},
{ "0bd9c7e0d1ea8800", 0x0},
{ "0231aabcac374528", 0x0},
{ "2e0c6f3882a17800", 0x0},
{ "5afb8b864d9956ba", 0x0},
{ "9034bb4c83284c00", 0x0},
{ "edf90eea8fa925c0", 0x0},
{ "cbd83ea69ffc38a0", 0x0},
{ "daa1c1ab826d5f00", 0x0},
{ "12ae7b18132e4cf0", 0x0},
{ "76154e76ed85d900", 0x0},
{ "d007ff96ee201d08", 0x0},
{ "e544ea261ca55000", 0x0},
{ "16a296230e9c4b44", 0x0},
{ "ba20ee3433a3b9d0", 0x0},
{ "95922aa1a77f6380", 0x0},
{ "6c09d35536e14fa0", 0x0},
{ "15dd8fb7b62a609e", 0x0},
{ "00e40fe533526e50", 0x0},
{ "4128e5f01ddbcad8", 0x0},
{ "4500dbc17beec800", 0x0},
{ "e713d16ffafa5382", 0x0},
{ "67e82c1ec4435670", 0x0},
{ "20a67ff404eadd60", 0x0},
{ "7a2628149559e580", 0x0},
{ "a29e06ae36e6b3d2", 0x0},
{ "8e5825fccfa36a80", 0x0},
{ "c3d901a491c6af7a", 0x0},
{ "7fbe581dd19b92f0", 0x0},
{ "5bd39bb487585b28", 0x0},
{ "e52ca8687fe4a38a", 0x0},
{ "fcedb97c5d05e660", 0x0},
{ "6cc6988c3635e080", 0x0},
{ "b2348d677b069878", 0x0},
{ "15f6de5e335b0000", 0x0},
{ "bb9932aceb5de100", 0x0},
{ "68792ee45417deb0", 0x0},
{ "a8cb07538ae186e0", 0x0},
{ "ba5c1b9e450e7839", 0x0},
{ "cb9c3b4ac8d7a1c0", 0x0},
{ "123191bfa022e188", 0x0},
{ "1526b3012e6bf8f0", 0x0},
{ "92f7a2546809ab58", 0x0},
{ "76b5aef85a0395c0", 0x0},
{ "1ca3891852022828", 0x0},
{ "ed12eed3eb1bc7a0", 0x0},
{ "ddb7146634e97940", 0x0},
{ "0b535c92db93e5a0", 0x0},
{ "f21be8dbb7ff9386", 0x0},
{ "9a0626d5f9ef7780", 0x0},
{ "07eb26bb75d12800", 0x0},
{ "1743b772aa88fcd0", 0x0},
{ "c0bd74fe2134c9ea", 0x0},
{ "28f1d704bbdc6080", 0x0},
{ "e0f1226cff1f99e0", 0x0},
{ "67f9d34031faf330", 0x0},
{ "33f346d4e6aaafd0", 0x0},
{ "9c2cea0a41f13800", 0x0},
{ "ac48a9852f718e00", 0x0},
{ "f5bebc72de1df390", 0x0},
{ "09df1aa450aab1e0", 0x0},
{ "3aae6efdb3c9d8e8", 0x0},
{ "c335614640d8dbc0", 0x0},
{ "770180af3a9fbf56", 0x0},
{ "3f17f98307250d00", 0x0},
{ "ffed5c37ae6c5340", 0x0},
{ "eb6686f283af7b8c", 0x0},
{ "ecd37969ee6b97c0", 0x0},
{ "6a2a7b29e1730e80", 0x0},
{ "1bc4b9ef52f7b1c0", 0x0},
{ "ab34661f19fdae80", 0x0},
{ "c30cc79c7a74fcc0", 0x0},
{ "3cadf48394f33dc4", 0x0},
{ "0ca1d47c04f49d00", 0x0},
{ "8567024f98d9de5b", 0x0},
{ "1820321658936000", 0x0},
{ "f25e95cd59803756", 0x0},
{ "3f9477acba1b2540", 0x0},
{ "54e0a91d1e9c35c0", 0x0},
{ "c6fd1f1e3506f200", 0x0},
{ "3940f43a7b82c140", 0x0},
{ "e14feadbeb7ff840", 0x0},
{ "86e2eaa7371089a0", 0x0},
{ "36740218f74e2bb8", 0x0},
{ "479abd77adc3eee8", 0x0},
{ "256fe4741566da28", 0x0},
{ "5bc34ed80a822a30", 0x0},
{ "1302074b483f4c00", 0x0},
{ "b29a6b6153711d60", 0x0},
{ "21690332e8e65400", 0x0},
{ "117e0d2df8a78230", 0x0},
{ "7680e1747bf15c80", 0x0},
{ "503e34d9aaabb400", 0x0},
{ "c0c967c9ef3e0838", 0x0},
{ "4ce505e7d9cc9126", 0x0},
{ "65a11bebe6475b38", 0x0},
{ "6f61959471208b20", 0x0},
{ "25d3b89ee81e6200", 0x0},
};

static int nregresell=1001;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Tstat						*thestats=NULL;
static char					key[1024]="";
static Asection				section;
static bool					ok=true;
static bool					running=false;

static void					launch();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline bool resellVerifyPassword(char *name, char *passwd)
{
	int		i;
	if(strlen(name)<5)
		return 0;
		
	for(i=0; i<nregresell; i++)
	{
		if(!strcmp(passwd, regresell[i].guid))
		{
			if(regfile[i].numname)
			{
				if(calcN(name)==regfile[i].numname)
					return (bool)-1;
			}
			else
				return (bool)-1;
			return 0;
		}
	}
	return 0;
}

bool VerifyPassword(char *name, char *passwd)
{
	if(resellVerifyPassword(name, passwd))
	{
		section.enter(__FILE__,__LINE__);
		strcpy(key, passwd);
		section.leave();
		launch();
		if(ok)
			return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline int	calcNotZero(char *str)
{
	int		v=0x12345678;
	int		i;
	char	*s=str;
	for(i=0; i<32; i++)
	{
		v^=*s;
		v<<=1;
		if(*s)
			s++;
		else
			s=str;
	}
	return v;
}


static bool checkID()
{
	bool	okfunc=false;
	//char	md5id[1024]="";
	char	id[1024]="";
	char	ke[1024]="";

	section.enter(__FILE__,__LINE__);
	strcpy(ke, key);
	section.leave();

	if(thestats->reseller==calcNotZero(ke))
		return true;

	if(getComputerInfo(id))
	{
//		char	toto[1024];
//		toto[0]=0;
//		strcpy(toto, "this is aestesis");
//		strcat(toto, id);
//		getMD5(md5id, toto);
	}
	else
		return false;

	WSADATA		wd;
	char		hostname[128];
	
	hostname[8]='.';
	hostname[3]='t';
	hostname[0]='a';
	hostname[2]='s';
	hostname[10]='r';
	hostname[7]='s';
	hostname[4]='e';
	hostname[11]='g';
	hostname[5]='s';
	hostname[6]='i';
	hostname[1]='e';
	hostname[9]='o';
	hostname[12]=0;

	if(WSAStartup(MAKEWORD(2, 2), &wd)==0)
	{
		unsigned long	addr=inet_addr(hostname);
		if(addr==INADDR_NONE)
		{		
			HOSTENT		*host=gethostbyname(hostname);
			if(host)
				if(host->h_addrtype==AF_INET)
					memcpy(&addr, host->h_addr, mini(host->h_length, sizeof(addr)));
		}

		if(addr!=INADDR_NONE)
		{
			IN_ADDR	*inaddr=(IN_ADDR *)&addr;
			SOCKET	s=socket(AF_INET, SOCK_STREAM, 0);

			if(s!=SOCKET_ERROR)
			{
				SOCKADDR_IN		sain;
				word			port=80;

				sain.sin_family	= AF_INET;
				sain.sin_addr	= *inaddr;
				sain.sin_port	= htons(port);

				if(connect(s, (SOCKADDR *)&sain, sizeof(sain))==0)
				{
					char	buffer[65536];
					int		length;

					sprintf(buffer, "GET /reseller.php?key=%s&id=%s HTTP/1.0\r\nHost: %s:%d\r\n", ke, id, hostname, port);
					strcat (buffer, "User-Agent: elektronika web client\r\n");
					strcat (buffer, "\r\n");
					strcat (buffer, "\r\n");

					length = strlen(buffer);
					if(send(s, buffer, length, 0)==length)
					{
						int				r=-1;
						int				t=0;
						unsigned long	vcmd=1;
						int				timeout=200;

						ioctlsocket(s, FIONBIO, &vcmd);

						while(timeout--)
						{
							r=recv(s, buffer+t, sizeof(buffer)-t, 0);
							if(r!=SOCKET_ERROR)
								t+=r;
							if((r==0)||(t>=sizeof(buffer)))
								break;

							Athread::sleep(5);
						}
						buffer[t]=0;

						if(r==0)
						{
							char	*str=buffer;
							int		len=0;
							while(str&&*str&&((str-buffer)<t))
							{
								if((str[0]=='\r')&&(str[1]=='\n'))
								{
									if(len==0)
										break;
									len=0;
									str++;
								}
								else if(str[0]!='\n')
									len++;
								str++;
							}
							
							len=t-(str-buffer);
							if(str&&(*str))
							{
								strupr(str);
								if(strstr(str, id))
								{
									thestats->reseller=calcNotZero(ke);
									okfunc=true;
								}
							}
						}

						vcmd=0;
						ioctlsocket(s, FIONBIO, &vcmd);
						
					}
				}
				closesocket(s);
			}
		}
	    WSACleanup();
	}
	return okfunc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AcheckIt : public Athread
{
public:
								AcheckIt();

	void						run();
};

AcheckIt::AcheckIt()
{
}

void AcheckIt::run()
{
	running=true;
	ok=checkID();
	sleep(200);
	running=false;
}

static AcheckIt	checkIt;

static void launch()
{
	if(!running)
		checkIt.start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif