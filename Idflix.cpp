#include <iostream>
using namespace std;

struct video {
    string judul;
    int durasi;
    string status;
    video *kiri, *kanan;
};

struct antrian {
    video* data;
    antrian* next;
};

struct riwayat {
    video* data;
    riwayat* next;
};

struct undo {
    string aksi;
    string judul;
    int durasi;
    string prevstatus;
    undo* next;
};

video* root = NULL;
antrian *depan = NULL, *belakang = NULL;
riwayat *atas = NULL;
undo *Top = NULL;

void buatplaylist() {
    depan = belakang = NULL;
}

void buatriwayat() {
    atas = NULL;
}

void buatundo() {
    Top = NULL;
}

bool duplikat(video* r, string j) {
    if (!r) return false;
    if (r->judul == j) return true;
    return j < r->judul ? duplikat(r->kiri, j) : duplikat(r->kanan, j);
}

video* tambah(video* r, string j, int d) {
    if (duplikat(r, j)) {
        cout << "Judul video sudah ada!\n";
        return r;
    }

    undo* new_undo = new undo;
    new_undo->aksi = "tambah";
    new_undo->judul = j;
    new_undo->durasi = d;
    new_undo->prevstatus = "tersedia";
    new_undo->next = Top;
    Top = new_undo;

    video* baru = new video;
    baru->judul = j;
    baru->durasi = d;
    baru->status = "tersedia";
    baru->kiri = baru->kanan = NULL;

    if (!r) return baru;
    if (j < r->judul) r->kiri = tambah(r->kiri, j, d);
    else r->kanan = tambah(r->kanan, j, d);
    return r;
}

void tampilvideo(video* r) {
    if (!r) return;
    tampilvideo(r->kiri);
    cout << r->judul << " | " << r->durasi << " menit | " << r->status << endl;
    tampilvideo(r->kanan);
}

video* carivideo(video* r, string j) {
    if (!r) return NULL;
    if (r->judul == j) return r;
    return j < r->judul ? carivideo(r->kiri, j) : carivideo(r->kanan, j);
}

void tambahlaylist(string j) {
    video* v = carivideo(root, j);
    if (!v) {
        cout << "Video tidak ditemukan.\n";
        return;
    }
    if (v->status != "tersedia") {
        cout << "Video tidak bisa dimasukkan ke playlist, Status: " << v->status << endl;
        return;
    }

    antrian* baru = new antrian;
    baru->data = v;
    baru->next = NULL;

    if (!depan) {
        v->status = "sedang diputar";
        depan = belakang = baru;
    } else {
        v->status = "dalam antrean";
        belakang->next = baru;
        belakang = baru;
    }

    undo* u = new undo;
    u->aksi = "playlist";
    u->judul = v->judul;
    u->durasi = v->durasi;
    u->prevstatus = "tersedia";
    u->next = Top;
    Top = u;

    cout << "Video berhasil masuk ke playlist\n";
}

void nontonvideo() {
    if (!depan) {
        cout << "Playlist kosong\n";
        return;
    }

    video* v = depan->data;
    v->status = "tersedia";

    riwayat* r = new riwayat;
    r->data = v;
    r->next = atas;
    atas = r;

    undo* u = new undo;
    u->aksi = "tonton";
    u->judul = v->judul;
    u->durasi = v->durasi;
    u->prevstatus = "sedang diputar";
    u->next = Top;
    Top = u;

    antrian* hapus = depan;
    depan = depan->next;
    delete hapus;

    if (depan) depan->data->status = "sedang diputar";

    cout << "Video ditonton: " << v->judul << endl;
}

void riwayatvideo() {
    if (!atas) {
        cout << "Riwayat masih kosong\n";
        return;
    }

    riwayat* b = atas;
    while (b != NULL) {
        cout << "- " << b->data->judul << " | " << b->data->durasi << " menit\n";
        b = b->next;
    }
}

video* carisisikiri(video* r) {
    while (r && r->kiri != NULL) {
        r = r->kiri;
    }
    return r;
}

video* hapusvideo(video* r, string j) {
    if (!r) return NULL;

    if (j < r->judul) {
        r->kiri = hapusvideo(r->kiri, j);
    } else if (j > r->judul) {
        r->kanan = hapusvideo(r->kanan, j);
    } else {
        if (r->status != "tersedia") {
            char yakin;
            cout << "Video sedang " << r->status << ". Yakin hapus? (y/t): ";
            cin >> yakin;
            if (yakin != 'y' && yakin != 'Y') return r;
        }

        undo* u = new undo;
        u->aksi = "hapus";
        u->judul = r->judul;
        u->durasi = r->durasi;
        u->prevstatus = r->status;
        u->next = Top;
        Top = u;

        if (!r->kiri && !r->kanan) {
            delete r;
            return NULL;
        } else if (!r->kiri) {
            video* tmp = r->kanan;
            delete r;
            return tmp;
        } else if (!r->kanan) {
            video* tmp = r->kiri;
            delete r;
            return tmp;
        } else {
            video* ganti = carisisikiri(r->kanan);
            r->judul = ganti->judul;
            r->durasi = ganti->durasi;
            r->status = ganti->status;
            r->kanan = hapusvideo(r->kanan, ganti->judul);
        }
    }
    return r;
}

video* undoaksiakhir(video* r) {
    if (!Top) {
        cout << "Tidak ada aksi untuk di-undo\n";
        return r;
    }

    undo* u = Top;
    Top = Top->next;

    if (u->aksi == "tambah") {
        r = hapusvideo(r, u->judul);
        cout << "Undo: video yang ditambahkan dihapus kembali\n";
    } else if (u->aksi == "hapus") {
        r = tambah(r, u->judul, u->durasi);
        cout << "Undo: video yang dihapus dikembalikan\n";
    } else if (u->aksi == "playlist") {
        antrian *bantu = depan, *sebelum = NULL;
        while (bantu) {
            if (bantu->data->judul == u->judul) {
                if (!sebelum) depan = bantu->next;
                else sebelum->next = bantu->next;
                if (bantu == belakang) belakang = sebelum;
                bantu->data->status = u->prevstatus;
                delete bantu;
                break;
            }
            sebelum = bantu;
            bantu = bantu->next;
        }
        if (depan) depan->data->status = "sedang diputar";
        cout << "Undo: video dikeluarkan dari playlist\n";
    } else if (u->aksi == "tonton") {
        if (atas && atas->data->judul == u->judul) {
            video* v = atas->data;
            riwayat* hapus = atas;
            atas = atas->next;
            delete hapus;

            antrian* baru = new antrian;
            baru->data = v;
            baru->next = depan;
            depan = baru;
            v->status = "sedang diputar";

            cout << "video dikembalikan\n";
        }
    }

    delete u;
    return r;
}

int main() {
    buatplaylist();
    buatriwayat();
    buatundo();

    int pilih;
    string judul;
    int durasi;

    do {
        cout << "\n=== MENU IDLIX TUBE ===\n";
        cout << "1. Tambah Video\n";
        cout << "2. Tampilkan Daftar Video\n";
        cout << "3. Tambah ke Playlist\n";
        cout << "4. Tonton Video\n";
        cout << "5. Tampilkan Riwayat\n";
        cout << "6. Hapus Video\n";
        cout << "7. Undo\n";
        cout << "8. Keluar\n";
        cout << "Pilihan: ";
        cin >> pilih;

        switch(pilih) {
            case 1:
                cin.ignore();
                cout << "Judul video: ";
                getline(cin, judul);
                cout << "Durasi(menit): ";
                cin >> durasi;
                root = tambah(root, judul, durasi);
                break;
            case 2:
                tampilvideo(root);
                char pilihCari;
                  cout << "\nApakah Anda ingin mencari video? (y/t): ";
                  cin >> pilihCari;

                if (pilihCari == 'y' || pilihCari == 'Y') {
                  cin.ignore();
                  cout << "Masukkan judul yang dicari: ";
                getline(cin, judul);
                video* hasil = carivideo(root, judul);
                    if (hasil != NULL) {
                        cout << "Video ditemukan:\n";
                        cout << hasil->judul << " | " << hasil->durasi << " menit | " << hasil->status << endl;
                    } else {
                        cout << "Video tidak ditemukan.\n";
                    }
                } else {
                cout << "Kembali ke menu utama.\n";
                }
    break;
                break;
            case 3:
                cin.ignore();
                cout << "Judul video: ";
                getline(cin, judul);
                tambahlaylist(judul);
                break;
            case 4:
                nontonvideo();
                break;
            case 5:
                riwayatvideo();
                break;
            case 6:
                cin.ignore();
                cout << "Judul video yang ingin dihapus: ";
                getline(cin, judul);
                root = hapusvideo(root, judul);
                break;
            case 7:
                root = undoaksiakhir(root);
                break;
            case 8:
                cout << "Terima kasih telah menggunakan IDLIX Tube!\n";
                break;
            default:
                cout << "Pilihan tidak valid!\n";
        }
    } while (pilih != 8);

    return 0;
}
