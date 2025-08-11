#include "inc/timertc.h"
#include "inc/config.h"
#include "pico/util/datetime.h"
#include "pico/stdlib.h"
#include "inc/wifi.h"
#include "lwip/dns.h"
#include "lwip/apps/sntp.h"

static volatile bool sntp_config_pending = true; // Flag to indicate if SNTP configuration is still pending
static volatile bool sntp_dns_successful = false; // Flag to indicate if SNTP DNS resolution was successful
bool rtc_initialized = false; // Flag to indicate if RTC has been initialized successfully

/**
 * @brief Sets the RTC time from SNTP epoch seconds and microseconds.
 *
 * This function converts the provided epoch seconds and microseconds
 * into a datetime_t structure and sets the RTC with this time.
 * 
 * It uses gmtime to convert the epoch seconds into a UTC time structure.
 *
 * @param epoch_seconds The number of seconds since the epoch (1970-01-01 00:00:00 UTC).
 * @param epoch_microseconds The number of microseconds to add to the epoch seconds.
 */

void my_rtc_set_from_sntp(uint32_t epoch_seconds, uint32_t epoch_microseconds)
{
    printf("my_rtc_set_from_sntp: Chamada com epoch %u s, %u us\n",
           (unsigned int)epoch_seconds, (unsigned int)epoch_microseconds);

    datetime_t dt = {0};
    time_t secs_for_gmtime = epoch_seconds;

    struct tm *utc_tm = gmtime(&secs_for_gmtime);

    if (utc_tm)
    {
        dt.year = (uint16_t)(utc_tm->tm_year + 1900);
        dt.month = (uint8_t)(utc_tm->tm_mon + 1);
        dt.day = (uint8_t)utc_tm->tm_mday;

        if (utc_tm->tm_wday == 0)
        {
            dt.dotw = 6;
        }
        else
        {
            dt.dotw = utc_tm->tm_wday - 1;
        }
        dt.hour = (uint8_t)utc_tm->tm_hour;
        dt.min = (uint8_t)utc_tm->tm_min;
        dt.sec = (uint8_t)utc_tm->tm_sec;

        printf("my_rtc_set_from_sntp: Convertido para %04d-%02d-%02d %02d:%02d:%02d (DOTW: %d)\n",
               dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec, dt.dotw);

        if (rtc_set_datetime(&dt))
        {
            printf("my_rtc_set_from_sntp: RTC configurado com SUCESSO pela função manual!\n");
        }
        else
        {
            printf("my_rtc_set_from_sntp: FALHA ao configurar RTC.\n");
        }
    }
    else
    {
        printf("my_rtc_set_from_sntp: gmtime() falhou para o epoch %u. RTC não será configurado.\n", (unsigned int)epoch_seconds);
    }
}

/**
 * @brief Callback function for DNS resolution of SNTP server.
 *
 * This function is called when the DNS resolution for the SNTP server completes.
 * It sets the SNTP server address and initializes SNTP with the resolved IP address.
 * If the resolution fails, it uses a fallback IP address.
 *
 * @param name The name of the DNS entry (not used here).
 * @param ipaddr The resolved IP address, or NULL if resolution failed.
 * @param callback_arg Additional argument passed to the callback (not used here).
 */

static void sntp_dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    LWIP_UNUSED_ARG(name);
    LWIP_UNUSED_ARG(callback_arg);

    if (ipaddr != NULL)
    {
        printf("DNS para SNTP resolvido: %s\n", ipaddr_ntoa(ipaddr));
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setserver(0, ipaddr);
        sntp_init();
        sntp_dns_successful = true;
        printf("SNTP configurado com IP resolvido e inicializado.\n");
    }
    else
    {
        printf("Falha ao resolver DNS para SNTP (callback retornou NULL).\n");
        printf("Usando IP de fallback para SNTP.\n");
        ip_addr_t fallback_ip;

        ipaddr_aton("200.160.7.186", &fallback_ip);
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setserver(0, &fallback_ip);
        sntp_init();
        sntp_dns_successful = false;
        printf("SNTP configurado com IP de fallback e inicializado.\n");
    }
    sntp_config_pending = false;
}

/**
 * @brief Initializes the RTC and synchronizes it with SNTP.
 *
 * This function checks if the Wi-Fi is connected, initializes the RTC,
 * and configures SNTP with a DNS server. It waits for the DNS resolution
 * to complete and then attempts to synchronize the RTC with the SNTP server.
 * If the DNS resolution fails, it uses a fallback IP address for SNTP.
 */

void init_and_sync_rtc()
{

    if (is_wifi_connected())
    {
        rtc_init();

        ip_addr_t dns_server_addr;
        ipaddr_aton("8.8.8.8", &dns_server_addr);
        dns_setserver(0, &dns_server_addr);

        printf("Inicializando SNTP (com resolução DNS assíncrona)...\n");

        sntp_config_pending = true;
        sntp_dns_successful = false;

        ip_addr_t ntp_server_ip_placeholder;

        err_t err = dns_gethostbyname("pool.ntp.org", &ntp_server_ip_placeholder, sntp_dns_found_cb, NULL);

        if (err == ERR_OK)
        {

            printf("Servidor NTP encontrado no cache DNS. Callback chamado.\n");

            if (sntp_config_pending)
            {
                sntp_dns_found_cb("pool.ntp.org", &ntp_server_ip_placeholder, NULL);
            }
        }
        else if (err == ERR_INPROGRESS)
        {

            printf("Requisição DNS para SNTP em andamento... Aguardando callback.\n");
        }
        else
        {

            printf("Erro crítico (%d) ao iniciar consulta DNS para SNTP. Usando fallback imediatamente.\n", err);
            ip_addr_t fallback_ip;
            ipaddr_aton("200.160.7.186", &fallback_ip);
            sntp_setoperatingmode(SNTP_OPMODE_POLL);
            sntp_setserver(0, &fallback_ip);
            sntp_init();
            sntp_dns_successful = false;
            sntp_config_pending = false;
            printf("SNTP configurado com IP de fallback devido a erro inicial de DNS.\n");
        }

        printf("Aguardando configuração do SNTP via DNS callback...\n");
        int dns_wait_retries = 0;

        while (sntp_config_pending && dns_wait_retries < 150)
        {
            cyw43_arch_poll();
            sleep_ms(100);
            dns_wait_retries++;
        }

        if (sntp_config_pending)
        {
            printf("Timeout! Callback do DNS não foi chamado para SNTP. SNTP não pode sincronizar.\n");

            sntp_stop();
            return;
        }

        printf("Configuração do SNTP concluída (ou fallback acionado). Aguardando sincronização do RTC...\n");

        datetime_t t_check = {0, 0, 0, 0, 0, 0, 0};
        int sntp_sync_retries = 0;
        const int max_sntp_sync_retries = 20;

        while (sntp_sync_retries < max_sntp_sync_retries)
        {

            cyw43_arch_poll();

            rtc_get_datetime(&t_check);

            if (t_check.year > 2023)
            {
                printf("RTC sincronizado via SNTP!\n");
                char datetime_buf[256];
                t_check.hour -= GMT_M_3; // Adjust for GMT-3
                datetime_to_str(datetime_buf, sizeof(datetime_buf), &t_check);
                printf("Tempo atual do RTC: %s\n", datetime_buf);
                rtc_initialized = true; // RTC initialized successfully
                break;
            }

            sleep_ms(1000);
            sntp_sync_retries++;
            if (sntp_sync_retries % 5 == 0)
            {
                printf("Tentativa de sincronizacao SNTP %d/%d. RTC: %04d-%02d-%02d\n",
                       sntp_sync_retries, max_sntp_sync_retries, t_check.year, t_check.month, t_check.day);
            }
        }

        if (sntp_sync_retries == max_sntp_sync_retries)
        {
            printf("Falha ao sincronizar RTC via SNTP após %d tentativas.\n", max_sntp_sync_retries);
            printf("Ultima leitura do RTC: %04d-%02d-%02d %02d:%02d:%02d\n",
                   t_check.year, t_check.month, t_check.day, t_check.hour, t_check.min, t_check.sec);
        }

        if (!sntp_config_pending)
            sntp_stop();
    }
    else{
        return;
    }
}    