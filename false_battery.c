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
struct sock *nl_sk = NULL;

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

static struct power_supply *add_battery_supply;


static int __init hello_init(void){
    printk("Entering: %s\n", __FUNCTION__);
    struct netlink_kernel_cfg cfg = { .input = recieve_msg_handler, };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static int __exit hello_exit(void){
    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);

    return 0;
}

module_init(hello_init);
module_exit(hello_exit);
