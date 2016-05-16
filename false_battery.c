// Includes added for battery specific stuff
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/vermagic.h>
#include <linux/power_supply.h>


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/power_supply.h>
#include <linux/errno.h>
#include <net/sock.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <asm-generic/types.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_USER 31
#define MAX_BATTERIES 5

struct sock *nl_sk = NULL;
struct power_supply *fake_batteries[MAX_BATTERIES];


///////////////////////////////////////////
/// \section Structs
/// \abstract All of the structs I'll be using
///
//////////////////////////////////////////

static enum power_supply_property battery_props[] = {
    POWER_SUPPLY_PROP_STATUS,
    POWER_SUPPLY_PROP_CHARGE_TYPE,
    POWER_SUPPLY_PROP_HEALTH,
    POWER_SUPPLY_PROP_PRESENT,
    POWER_SUPPLY_PROP_TECHNOLOGY,
    POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
    POWER_SUPPLY_PROP_CHARGE_FULL,
    POWER_SUPPLY_PROP_CHARGE_NOW,
    POWER_SUPPLY_PROP_CAPACITY,
    POWER_SUPPLY_PROP_CAPACITY_LEVEL,
    POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG,
    POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
    POWER_SUPPLY_PROP_MODEL_NAME,
    POWER_SUPPLY_PROP_MANUFACTURER,
    POWER_SUPPLY_PROP_SERIAL_NUMBER,
};




static void hello_nl_recieve_msg(struct sk_buff *skb){
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from the kernel";
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    msg_size = strlen(msg);
    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received msg payload:%s\n", (char *)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid;

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out){
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to usr\n");

}


// Battery Functions as they are written in test_power.c
static int get_battery_property(struct power_supply *psy,
                                enum power_supply_property psp,
                                union power_supply_proval *val){
    return 0;

}

static int numBatteries = 0;

static int remove_all_battery_supplies(void){
    if(numBatteries == 0) return 0;
    int i = numBatteries;
    while(i -->=0){
        power_supply_unregister(fake_batteries[numBatteries]);
        numBatteries--;
    }
}


static int add_battery_supply (const struct power_supply_config *config,
                               const struct power_supply_desc *description){

    printk(KERN_INFO "INSIDE: %s", __FUNCTION__);
    if(numBatteries >= MAX_BATTERIES){
        printk(KERN_INFO "Max Number of batteries added");
        return -1;
    }

    fake_batteries[numBatteries] = power_supply_register(NULL, description, config);
    if(IS_ERR(fake_batteries[numBatteries])){
        printk(KERN_ERR "Failed to create a battery\n");
        return -2;
    }

    printk(KERN_INFO "Number of Batteries: %d\n", ++numBatteries);
    return 0;

    // power_supply_register( NULL, description, config);
}

static int make_battery(void){
    // TODO turn into one init function
    static struct power_supply_desc battery_description;
    battery_description.name            = "test_battery";
    battery_description.type            = POWER_SUPPLY_TYPE_BATTERY;
    battery_description.properties      = battery_props;
    battery_description.num_properties  = ARRAY_SIZE(battery_props);
    battery_description.get_property    = get_battery_property;

    static struct power_supply_config battery_config = {};

    return add_battery_supply(&battery_description, &battery_config);

}

static void recieve_msg_handler(struct sk_buff *skb){
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from the kernel";
    int res;

    // This is an amazing piece of code.
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    msg_size = strlen(msg);
    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received msg payload:%s\n", (char *)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid;

    // This is what is getting sent out is generated.
    // nlmsg_new takes a size for a buffer
    skb_out = nlmsg_new(msg_size, 0);

    /////////////////////////////////////
    /// Adding my own function here. Will be executed from here

    if(make_battery() < 0){
        msg = "ERROR: Max Batteries Achieved";
        msg_size = strlen(msg);
        skb_out = nlmsg_new(msg_size, 0);
        if (!skb_out){
            printk(KERN_ERR "Failed to allocate new skb\n");
            return;
        }

    }

    //////////////////////////////////////
    /// Sending of the messages here
    ///
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to usr\n");

}

//////////////////////////////////////
/// Kernel Endpoints
///
///
static int __init hello_init(void){
    printk("Entering: %s\n", __FUNCTION__);
    struct power_supply *fake_batteries[MAX_BATTERIES];
    struct netlink_kernel_cfg cfg = { .input = recieve_msg_handler, };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static int __exit hello_exit(void){
    printk(KERN_INFO "Leaving Battery Module. Cleaning up made power supplies");
    printk(KERN_INFO "Number of batteries to remove: %d", numBatteries);
    int removal = remove_all_battery_supplies();
    printk(KERN_INFO "LEAVING WITH %d BATTERIES", numBatteries);
    netlink_kernel_release(nl_sk);

    return 0;
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_DESCRIPTION("Battery Driver for virtual batteries with netlink control.");
MODULE_AUTHOR("Irving Derin <Irving.Derin@gmail.com>");
MODULE_LICENSE("GPL");
