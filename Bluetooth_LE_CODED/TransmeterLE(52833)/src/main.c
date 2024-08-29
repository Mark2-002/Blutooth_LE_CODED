#include <zephyr/bluetooth/bluetooth.h> // header files for the project
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/pm/pm.h>
#include <hal/nrf_gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>

struct bt_le_ext_adv *adv; // Declaration of a pointer variable named 'adv'
LOG_MODULE_REGISTER(TRANSMETER,LOG_LEVEL_DBG);
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)


static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led12 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led22 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

int err;
/* Button variables */
static uint32_t time, last_time; /* used for detecting long button press */
// static uint32_t button_counter = 0; // Static variable 'button_counter' of type uint32_t (unsigned 32-bit integer) initialized to 0
int button_tick = 0;  /* Integer variable 'button_tick' initialized to 0 */
int sleep_flag = 1;   /* Integer variable 'button_tick' initialized to 0 */
int counter = 1;     /*  Integer variable 'counter' initialized to 1 */

//---for le coded
struct bt_le_adv_param adv_param = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_USE_IDENTITY | BT_LE_ADV_OPT_CODED,
                                                        0x30, //  0x12C0,
                                                        0x30, // 0x12C0,  //advertising interval is set to 3 seconds

                                                        NULL);

struct bt_le_ext_adv_start_param ext_adv_param = BT_LE_EXT_ADV_START_PARAM_INIT(0, 1); // number of events is set to 2.
static uint8_t mfg_data[8]={0};  
  
                                                    // Static array 'mfg_data' of type uint8_t (unsigned 8-bit integer) with 8 elements, initialized to 0
static const struct bt_data ad[] = {
    BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 8), // Static constant array 'ad' of struct bt_data type, containing a single element
};

 /* function for creating a random static address and setting it as the identity address of the device. */
static void set_random_static_address(void) // Print a message indicating the start of the iBeacon Demo
{

        printk("::::::::::::::Starting LE-CODED Transmeter:::::::::::\n");  /* Print a message indicating the start of the iBeacon Demo */

        bt_addr_le_t addr; // Declaration of a Bluetooth Low Energy address structure variable named 'addr'
        int err;
        // err = bt_addr_le_from_str("DE:AD:BE:AF:BA:11", "random", &addr);      /* public address for check */
        err = bt_addr_le_from_str("FA:17:DF:DF:4B:89", "random", &addr);        // gateway address
        // err = bt_addr_le_from_str("CE:AD:BE:AF:BA:11","random" , &addr);     // gateway address
        // err = bt_addr_le_from_str("DE:AD:BE:AF:BA:11", "random", &addr);     // device address

        if (err)
        {
                printk("Invalid BT address (err %d)\n",err); // Print an error message with the error code
        }

        /* create a new Identity address. This must be done before enabling Bluetooth. Addr is the address used for the new identity */
        err = bt_id_create(&addr, NULL); /*  Create a new Bluetooth ID using the provided address (addr) and no additional parameters */

        if (err < 0) /* Check if error occurred during initialization */
        {
                printk("Creating new ID failed (err %d)\n", err); /*  Print an error message if creating the new ID failed */
        }
        printk("Created new address\n");  /* Print a message indicating that a new address has been successfully created */
}
void bt_data(){
        mfg_data[1]=450;  

}

 /* function to create the extended advertising set using the advertising parameters. */
void adv_param_init(void)
{
        int err;  /* Declaration of the error variable to store return codes or error status */

        err = bt_le_ext_adv_create(&adv_param, NULL, &adv); // /* Create a new extended advertising instance and store the return code in 'err' */

        if (err) // /* Check if an error occurred during extended advertising creation */
        {
                printk("Failed to create advertising set (err %d)\n", err); // /* Print an error message indicating failed advertising set creation with the specific error code */
                return;                                                     // /* Exit the function early due to the failed advertising set creation */
        }
        printk("Created extended advertising set \n"); // /* Print a message indicating successful creation of an extended advertising set */
}

void start_adv(void)
{       

        mfg_data[0]=counter;
        bt_data();
        err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0); /* Set extended advertising data for the advertising instance 'adv' using the advertisement data 'ad' array */
        if (err)                                                        /* Check if an error occurred while setting extended advertising data */
        {
                printk("Failed (err %d)\n", err); // Print an error message indicating the failure and include the error code in the message
                return;                           // Exit the current function due to the failure in the previous operation
        }
        printk("Start Extended Advertising...\n");  
        printk("%s Is sent to receiver",mfg_data);     // Print a message indicating the start of extended advertising
        
        err = bt_le_ext_adv_start(adv, &ext_adv_param); // BT_LE_EXT_ADV_START_DEFAULT);

        if (err) /* Check if an error occurred while setting extended advertising data */
        {
                printk("Failed to start extended advertising " // Print a message indicating the failing of the start extended advertising
                       "(err %d)\n",
                       err);
                return; // Exit the current function due to the failure in the previous operation
        }
        printk("done.\n");

        nrf_gpio_pin_set(DT_GPIO_PIN(DT_NODELABEL(led0), gpios)); 
        gpio_pin_toggle_dt(&led22);
        nrf_gpio_pin_toggle(DT_GPIO_PIN(DT_NODELABEL(led1), gpios));
        k_msleep(500); // delay time
        // K_MSEC(500);
        bt_le_ext_adv_stop(adv);                /*  Stop extended advertising for the advertising instance 'adv' */
        printk("Stopped advertising..!!\n");    /*  Print a message indicating that advertising has been stopped */
        
        counter++;
        start_adv(); 
          
                                  /* Call a function named 'start_adv' to initiate advertising again */
}

int j = 0;

void main() 
{

 
        nrf_gpio_cfg_output(DT_GPIO_PIN(DT_NODELABEL(led1), gpios));
        /* Create a random static address and set it as the identity address of the device */
        set_random_static_address();

        /* Initialize the Bluetooth Subsystem */
        err = bt_enable(NULL); // Enable Bluetooth functionality with default parameters
        if (err)
        {
                printk("Bluetooth init failed (err %d)\n", err); // print a  message indicating that bluetooth initialization is failed
                return;
        }
        adv_param_init(); // Initialize advertising parameters
        start_adv();      // Start advertising
}