# Sosyal Ağ Analiz Sistemi

Bu proje, temel sosyal ağ analiz işlemlerini gerçekleştirmek için C programlama dilinde geliştirilmiş bir sistemdir. Kullanıcıların ve arkadaşlık ilişkilerinin bir dosyadan okunmasıyla oluşturulan sosyal ağ üzerinde çeşitli analizler yapılabilmektedir.

## Özellikler

* **İlişki Ağacı Oluşturma:** Belirli bir kullanıcıyı kök alarak ve istenen derinliğe kadar arkadaşlık ilişkilerini görselleştiren bir ağaç yapısı oluşturur.
* **Belirli Mesafedeki Arkadaşları Bulma:** Seçilen bir kullanıcının belirli bir derinlikteki (mesafedeki) arkadaşlarını listeler.
* **Ortak Arkadaş Analizi:** İki kullanıcının ortak arkadaşlarını bulur ve listeler.
* **Topluluk Tespiti:** Basit bir genişlik öncelikli arama (BFS) algoritması kullanarak sosyal ağdaki bağlantılı toplulukları tespit eder ve gösterir.
* **Kullanıcı Etki Alanı Hesaplama:** Bir kullanıcının ağ üzerindeki etkisini, arkadaşlarının bağlantı sayısı üzerinden basit bir metrikle hesaplar.
* **Kırmızı-Siyah Ağaçta Kullanıcı Arama:** Kullanıcıları ID'lerine göre hızlı bir şekilde aramak için Kırmızı-Siyah ağaç veri yapısını kullanır.

## Nasıl Çalıştırılır

  **Gereksinimler:**
    * Bir C derleyicisi (örneğin GCC)
    * Gerekli standart C kütüphaneleri


    ```

  **Veri Dosyası Oluşturma:**
    Proje dizininde `veriseti.txt` adında bir metin dosyası oluşturun. Kullanıcıları ve arkadaşlık ilişkilerini aşağıdaki formatta bu dosyaya ekleyin:

    * **Kullanıcı Ekleme:** `USER <ID> <İsim>`
        ```
        USER 1 Ayşe
        USER 2 Burak
        USER 3 Can
        ```
    * **Arkadaşlık Ekleme:** `FRIEND <Kullanıcı ID 1> <Kullanıcı ID 2>`
        ```
        FRIEND 1 2
        FRIEND 1 3
        FRIEND 2 3
        ```



## Veri Yapıları

* **`User`:** Kullanıcı bilgilerini (ID ve isim) tutan yapı.
* **`GraphNode`:** Grafın komşuluk listesi için kullanılan düğüm yapısı. Her kullanıcının arkadaşları bu liste ile tutulur.
* **`Graph`:** Sosyal ağı temsil eden ana yapı. Kullanıcı dizisi (`users`), komşuluk listesi dizisi (`edges`) ve kullanıcı sayısını (`userCount`) içerir.
* **`RelationTreeNode`:** İlişki ağacını oluşturmak için kullanılan düğüm yapısı. Kullanıcı ID'si, derinlik ve çocuk/kardeş düğümlerine yönelik işaretçiler içerir.
* **`RBTreeNode`:** Kırmızı-Siyah ağacını temsil eden düğüm yapısı. Kullanıcı bilgisi, renk ve sol/sağ çocuk/ebeveyn işaretçilerini içerir.

## Algoritmalar

* **Genişlik Öncelikli Arama (BFS):** İlişki ağacı oluşturma ve topluluk tespiti için kullanılır.
* **Derinlik Öncelikli Arama (DFS):** Belirli mesafedeki arkadaşları bulmak için kullanılır.
* **Kırmızı-Siyah Ağaç:** Kullanıcıları ID'lerine göre verimli bir şekilde saklamak ve aramak için kullanılır.

