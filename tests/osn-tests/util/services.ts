const request = require('request');

type TPlatform = 'twitch' | 'youtube' | 'mixer' | 'facebook';

interface IPlatformAuth {
    widgetToken: string;
    apiToken: string; // Streamlabs API Token
    platform: {
      type: TPlatform;
      username: string;
      token: string;
      id: string;
      channelId?: string;
    };
}

interface ITestUser {
    name: string; // must be unique
    workerId: string; // null if user is not active right now
    updated: string; // time of the last request for this user
    platforms: {
      // tokens for platforms
      username: string; // Mixer use username as an id for API requests
      type: TPlatform; // twitch, youtube, etc..
      id: string; // platform userId
      token: string; // platform token
      apiToken: string; // Streamlabs API token
      widgetToken: string; // needs for widgets showing
      channelId?: string; // for the Mixer and Facebook only
    }[];
}

export class Services {
    private username: string = '';
    private user: ITestUser;
    private authInfo: IPlatformAuth;
    private userPoolUrl: string = 'https://slobs-users-pool.herokuapp.com/';
    private clientId = '8bmp6j83z5w4mepq0dn0q1a7g186azi';

    private sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    private async requestUser(): Promise<any> {
        return new Promise((resolve, reject) => {
            request({
                url: this.userPoolUrl + 'reserve',
                headers: {Authorization: `Bearer: ${process.env.SLOBS_TEST_USER_POOL_TOKEN}`}},
                (err: any, res: any, body: any) => {
                    if (err || res.statusCode !== 200) {
                        reject(`Unable to request user ${err || body}`);
                    }

                    resolve(JSON.parse(body));
                }
            );
        });
    }

    private async validateToken(token: string) {
        return new Promise((resolve, reject) => {
            request({
                url: 'https://id.twitch.tv/oauth2/validate',
                headers: {'Client-id': this.clientId,
                          Accept: 'application/vnd.twitchtv.v5+json',
                          'Content-Type': 'application/json',
                          Authorization: `OAuth ${token}`}
            }, (err: any, res: any, body: any) => {
                if (err || res.statusCode !== 200) {
                    reject(`Unable to validate token ${err || body}`);
                }

                resolve(JSON.parse(body));
            });
        });
    }

    private async requestStreamKey(token: string) {
        return new Promise((resolve, reject) => {
            request({
                url: 'https://api.twitch.tv/kraken/channel',
                headers: {'Client-id': this.clientId,
                          Accept: 'application/vnd.twitchtv.v5+json',
                          'Content-Type': 'application/json',
                          Authorization: `OAuth ${token}`}
            }, (err: any, res: any, body: any) => {
                if (err || res.statusCode !== 200) {
                    reject(`Unable to get channel info ${err || body}`);
                }

                resolve(JSON.parse(body));
            });
        });
    }

    async getStreamKey(lsPlatform: TPlatform): Promise<string> {
        let attemps: number = 3;

        if (this.username.length > 0) {
            throw 'User already being used.';
        }

        while(attemps--) {
            try {
                this.user = await this.requestUser();
            } catch(e) {
                if (attemps) {
                    await this.sleep(20000);
                }
            }
        }

        if (!this.user) {
            throw 'Unable to get user from pool.';
        }

        this.username = this.user.name;
        const token = this.user.platforms.find(platform => {
            return platform.type === lsPlatform;
        }).token;

        let channelInfo: any;

        if (lsPlatform == 'twitch') {
            await this.validateToken(token);
            channelInfo = await this.requestStreamKey(token);
        }

        return channelInfo.stream_key;
    }

    async releaseUser() {
        return new Promise((resolve, reject) => {
            request({
                url: this.userPoolUrl + `release/${this.username}`,
                headers: {Authorization: `Bearer: ${process.env.SLOBS_TEST_USER_POOL_TOKEN}`}
            }, (err: any, res: any, body: any) => {
                if (err || res.statusCode !== 200) {
                    reject(`Unable to release user ${err || body}`);
                }

                this.username = '';
                resolve(JSON.parse(body));
            });
        });
    }
}