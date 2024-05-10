package crawler;

import info.kgeorgiy.java.advanced.crawler.*;

import java.io.IOException;
import java.util.*;
import java.util.concurrent.*;

/**
 * Crawls websites, filtering <a href="http://tools.ietf.org/html/rfc3986">URLs</a> by substrings.
 *
 */
public class WebCrawler implements NewCrawler {
    private final Downloader downloader;
    private final ExecutorService download;
    private final ExecutorService extract;

    public WebCrawler(Downloader downloader, int downloads, int extractors, int perHost) {
        this.downloader = downloader;
        this.download = Executors.newFixedThreadPool(downloads);
        this.extract = Executors.newFixedThreadPool(extractors);
    }

    private boolean checkExcluded(String str, Set<String> excludes) {
        return excludes.stream().anyMatch(str::contains);
    }

    private void extractTask(Phaser ph, ConcurrentLinkedQueue<String> nextLevel, String current,
                             Set<String> excludes, Set<String> visited, Map<String, IOException> errors,
                             Document document) {
        ph.register();
        extract.submit(() -> {
            try {
                List<String> links = document.extractLinks();
                for (String link : links) {
                    if (checkExcluded(link, excludes)) continue;
                    if (!errors.containsKey(link) && visited.add(link)) {
                        nextLevel.add(link);
                    }
                }
            } catch (IOException e) {
                errors.put(current, e);
            } finally {
                ph.arriveAndDeregister();
            }
        });
    }

    private void downloadTask(boolean isNotLast, Phaser ph, ConcurrentLinkedQueue<String> nextLevel,
                              String current, Set<String> excludes, Set<String> visited,
                              Map<String, IOException> errors) {
        ph.register();
        download.submit(() -> {
            try {
                Document document = downloader.download(current);
                if (!errors.containsKey(current)) {
                    visited.add(current);
                    if (isNotLast) {
                        extractTask(ph, nextLevel, current, excludes, visited, errors, document);
                    }
                }
            } catch (IOException e) {
                visited.remove(current);
                errors.put(current, e);
            } finally {
                ph.arriveAndDeregister();
            }
        });
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Result download(String url, int depth, Set<String> excludes) {
        if (depth == 0 || url == null || checkExcluded(url, excludes)) {
            return new Result(Collections.emptyList(), Collections.emptyMap());
        }

        Phaser ph = new Phaser(1);
        Set<String> visited = ConcurrentHashMap.newKeySet();
        Map<String, IOException> errors = new ConcurrentHashMap<>();
        ConcurrentLinkedQueue<String> toVisit = new ConcurrentLinkedQueue<>();
        toVisit.add(url);
        visited.add(url);

        for (int i = 0; i < depth; i++) {
            ConcurrentLinkedQueue<String> nextLevel = new ConcurrentLinkedQueue<>();
            for (String current : toVisit) {
                boolean flag = i < depth - 1;
                downloadTask(flag, ph, nextLevel, current, excludes, visited, errors);
            }
            ph.arriveAndAwaitAdvance();
            toVisit = nextLevel;
        }
        ph.arriveAndDeregister();
        List<String> downloaded = new ArrayList<>(visited);
        return new Result(downloaded, errors);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void close() {
        // close
        download.shutdown();
        extract.shutdown();
        try {
            int WAITING_TIME = 30;
            if (!download.awaitTermination(WAITING_TIME, TimeUnit.DAYS)) {
                download.shutdownNow();
            }
            if (!extract.awaitTermination(WAITING_TIME, TimeUnit.DAYS)) {
                extract.shutdownNow();
            }
        } catch (InterruptedException e) {
            download.shutdownNow();
            extract.shutdownNow();
        }
    }

    /**
     * The main method for the WebCrawler.
     *
     * @param args Expected arguments are:
     *             url - the URL to start crawling from.
     *             depth - the maximum depth.
     *             downloads - the number of download threads.
     *             extractors - the number of extractor threads.
     *             perHost - the maximum number of connections per host (perHost >= downloads because easy version)
     */
    public static void main(String[] args) {
        if (args == null || args.length != 5 || Arrays.stream(args).anyMatch(Objects::isNull)) {
            System.out.println("Usage: WebCrawler url [depth [downloads [extractors [perHost]]]]");
            return;
        }
        String url = args[0];
        int depth, downloads, extractors, perHost;
        try {
            depth = Integer.parseInt(args[1]);
            downloads = Integer.parseInt(args[2]);
            extractors = Integer.parseInt(args[3]);
            perHost = Integer.parseInt(args[4]);
        } catch (NumberFormatException e) {
            System.out.println("Usage: WebCrawler url [depth [downloads [extractors [perHost]]]]");
            return;
        }
        try {
            try (WebCrawler crawler = new WebCrawler(new CachingDownloader(1), downloads, extractors, perHost)) {
                crawler.download(url, depth, new HashSet<>());
            }
        } catch (IOException e) {
            System.err.println("Error while downloading: " + e.getMessage());
        }
    }
}
